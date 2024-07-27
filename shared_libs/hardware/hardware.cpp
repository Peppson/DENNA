
#include "hardware.h"
#include "utility.h"
#include "../shared_libs/rf24_radio/rf24_radio.h"


// Members
uint32_t Hardware::deepsleep_time = 0; 
uint32_t Hardware::next_update_time = 0;
RF24Radio* Hardware::_radio_ptr;
volatile bool _lightsleep_wakeup;


// RTC Interrupt handler. Wakes up the CPU periodically while deepsleeping
// Only way to timekeep while deepsleeping
ISR (RTC_CNT_vect) {
    RTC.INTFLAGS |= RTC_OVF_bm;
    _lightsleep_wakeup = true;
}


// Helper
constexpr uint32_t convert_hhmm_to_seconds(const uint32_t& value) {
    static_assert(selected_node::SLEEP_AT_THIS_TIME >= 0 && 
        selected_node::SLEEP_AT_THIS_TIME <= 2400, "Node time must follow \"hhmm\" format");

    uint32_t hour = value / 100;  
    uint32_t minute = value % 100; 

    return (hour * 60 * 60) + (minute * 60);
}


void Hardware::begin(RF24Radio* radio_ptr) {
    #if USB_SERIAL_ENABLED
        init_TX_serial(); 
    #endif

    // Helps with uploading if CPU is stuck "RSP_ILLEGAL_MCU_STATE"
    delay(2000);
    #if DEV_RECOVERY_DELAY
        delay(5000);
    #endif

    // Enable Watchdog at max interval ≈ 4s 
    WDT_ENABLE();
    WDT_FEED();

    // Set internal 2.5V reference for ADC readings
    ADC0.CTRLC = TIMEBASE_1US | INTERNAL2V5;

    // Save pointer to radio
    _radio_ptr = radio_ptr; 

    // Disable unused peripherals
    ADC0.CTRLA &= ~ADC_ENABLE_bm;   // ADC 
    TWI0.CTRLA &= ~TWI_ENABLE_bm;   // I2C
    CCL.CTRLA &= ~CCL_ENABLE_bm;    // CCL
}


void Hardware::set_default_pin_states(bool is_battery_Driven) {
    pinMode(PIN_POWER_IO, OUTPUT);
    digitalWrite(PIN_POWER_IO, LOW);
    pinMode(PIN_UPDI, INPUT_PULLUP);
    pinMode(PIN_TX_SERAIL, INPUT_PULLUP);
    pinMode(PIN_PA4, INPUT_PULLUP);
    pinMode(PIN_PA5, INPUT_PULLUP);
    pinMode(PIN_PA7, INPUT_PULLUP);

    is_battery_Driven ? pinMode(PIN_PA6, INPUT) : pinMode(PIN_PA6, INPUT_PULLUP); // Battery ADC pin
}


void Hardware::disable_all_pins() {
    // PIN_POWER_IO has to remain active to hold state low
    for (size_t i = 0; i < 12; i++) {
        if (i == PIN_POWER_IO) { continue; }
        pinMode(i, INPUT_PULLUP);
    }
}


void Hardware::init_TX_serial() {
    Serial.begin(SERIAL_BAUD_RATE, (SERIAL_8N1 | SERIAL_TX_ONLY));
    while (!Serial) { }
    log("\n-- Node %i --\n", RF24_THIS_ADDRESS);

    #if USB_SERIAL_ENABLED
        //#error "Makes sure nothing is connected on RX pin" // Even with TX only...
    #endif
}


void Hardware::restart_device() { 
    _radio_ptr->power_down();
    
    // Wait for Watchdog to reset board
    _PROTECTED_WRITE(WDT.CTRLA, WDT_PERIOD_4KCLK_gc);
    while (1) { delay(1); } 
}


void Hardware::deepsleep(uint32_t sleep_duration_s) {
    const uint16_t sleep_limit = sleep_duration_s / SLEEP_RTC_INTERVAL_SECONDS;
    volatile uint16_t sleep_counter = 0;

    disable_all_peripherals();
    WDT_DISABLE();

    //
    // Enable periodic RTC interrupts to allow the CPU to wake up from sleep
    // Average time between interrupts = 64.299 seconds
    // Continue to sleep until (sleep_counter * 64.299) > SLEEP_DURATION_SECONDS
    // Average current = 3uA while sleeping
    // 
  
    RTC.PER = 0xFFFF;                               // Set period: 1-65535 (0XFFFF longest possible interval)
    RTC.CLKSEL = RTC_CLKSEL_INT1K_gc;               // Set 1kHz internal clock as source
    RTC.INTCTRL |= RTC_OVF_bm;                      // Interrupt when period overflows
    RTC.CTRLA = RTC_RTCEN_bm | RTC_RUNSTDBY_bm;     // Enable + active in SLEEP_MODE_STANDBY

    // ZZZZzzzzZzzzzZzz
    set_sleep_mode(SLEEP_MODE_STANDBY);
    sleep_enable();
    sleep_cpu();

    // Continue to sleep for sleep_duration_s
    while (sleep_counter < sleep_limit) {
        sleep_counter++;    
        RTC.CNT = 0; // Reset timer
        sleep_cpu();
    }
        
    // Wake up
    sleep_disable();
    restart_device(); // Full reset
}


void Hardware::lightsleep(uint16_t sleep_duration_ms) {
    WDT_FEED();
    if (sleep_duration_ms >= 35*1000) { // Dont allow longer sleep than the watchdog timeout
        sleep_duration_ms = 35*1000; 
    }

    // Similar behavior as deepsleep()
    RTC.PER = sleep_duration_ms;            // Set period: 1-65535 
    RTC.CLKSEL = RTC_CLKSEL_INT1K_gc;       // Set 1kHz internal clock as source
    RTC.INTCTRL |= RTC_OVF_bm;              // Interrupt when period overflows
    RTC.CTRLA = RTC_RTCEN_bm;               // Enable
    RTC.CNT = 0;                            // Reset RTC

    // ZZZZzzzzZzzzzZzz
    _lightsleep_wakeup = false; 
    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();
    sleep_cpu();

    // Continue to lightsleep for sleep_duration_ms
    // TCB 0 and 1 interrupts alot, only wake up from RTC interrupt 
    while (!_lightsleep_wakeup) {    
        WDT_FEED();
        sleep_cpu();
    }

    // Wake up
    RTC.INTCTRL &= ~RTC_OVF_bm; // Disable RTC interrupt
    sleep_disable();
    WDT_FEED();
}


void Hardware::disable_all_peripherals() {
    if (_radio_ptr) {
        _radio_ptr->power_down();
        delay(2000); // Needed!
        _radio_ptr->~RF24Radio();
    }

    // Cpu
    ADC0.CTRLA &= ~ADC_ENABLE_bm;                   // ADC
    SPI0.CTRLA &= ~SPI_ENABLE_bm;                   // SPI 
    TWI0.CTRLA &= ~TWI_ENABLE_bm;                   // I2C
    CCL.CTRLA &= ~CCL_ENABLE_bm;                    // CCL (Configurable Custom Logic)
    TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;     // TCA0 timer
    TCB0.CTRLA &= ~TCB_ENABLE_bm;                   // TCB0 timer

    // Disable all pins
    disable_all_pins();
    
    // Current draw while sleeping with Serial enable = 3uA, about 80uA when disable
    // Only difference is pin PB2(TX) is set as an OUTPUT when using serial... 
    // Simple setting the pin as OUTPUT = -80uA wtf??
    pinMode(PIN_PB2, OUTPUT);
}


void Hardware::timeout(uint32_t timeout_ms) {
    uint32_t end_time = millis() + timeout_ms;

    while (millis() < end_time) {
        WDT_FEED();
        delay(10);
    }
}


uint16_t Hardware::get_remaining_battery_charge() { 
    constexpr uint8_t samples = 100;
    WDT_FEED();

    // Enable ADC
    ADC0.CTRLA |= ADC_ENABLE_bm;

    // Sum ADC readings (0-1023)
    uint32_t sum = 0;
    for (size_t i = 0; i < samples; i++) {
        uint16_t level = analogRead(PIN_PA6); 
        sum += constrain(level, 0, 1023);
        delay(1);
    }

    // Turn off ADC
    ADC0.CTRLA &= ~ADC_ENABLE_bm;

    return sum / samples;
}


bool Hardware::set_time_until_deepsleep(uint16_t time_input) { 
    if (!verify_time_input(time_input)) { 
        return false; 
    }

    // Convert to seconds
    constexpr uint32_t sleep_at_this_time = convert_hhmm_to_seconds(selected_node::SLEEP_AT_THIS_TIME);
    uint32_t current_time = convert_hhmm_to_seconds(time_input);

    // Is current time inside the "sleep window"? If so, how long should we sleep?
    // Case before midnight 
    if (current_time >= sleep_at_this_time) {
        uint32_t offset = current_time - sleep_at_this_time;
        uint32_t sleep_duration_s = selected_node::SLEEP_DURATION_SECONDS - offset;   
        deepsleep(sleep_duration_s);

    // Case after midnight
    } else if (current_time < sleep_at_this_time - selected_node::SLEEP_DURATION_SECONDS) {
        uint32_t sleep_duration_s = sleep_at_this_time - current_time - selected_node::SLEEP_DURATION_SECONDS;
        deepsleep(sleep_duration_s);

    // Outside "sleep window". Set time until "selected_node::SLEEP_AT_THIS_TIME"
    } else {
        uint32_t time_left_ms = (sleep_at_this_time - current_time) * 1000UL;   // Convert to milliseconds
        deepsleep_time = time_left_ms + millis();                               // Add current time for absolute timestamp
    }

    return true;
}


bool Hardware::verify_time_input(const uint16_t time) {
    return (time >= 0 && time <= 2400);
}


/* 
void Hardware::print_time(const uint32_t& time_left) {
    uint32_t hour = (time_left / (60 * 60)) % 24;
    uint32_t minute = (time_left / 60) % 60;

    log("hour left: %i\n", hour);
    log("minute left: %i\n", minute);
} 
*/


#pragma once
#include "../global_config.h"
class RF24Radio;

// RTC Interrupt handler. Wakes up the CPU periodically while deepsleeping
ISR (RTC_CNT_vect);


class Hardware {
public:
    Hardware(const bool is_battery_driven) { 
        set_default_pin_states(is_battery_driven);
    }
    
    static void begin(RF24Radio* radio_ptr);
    static void set_default_pin_states(const bool is_battery_Driven = false);
    static void disable_all_pins();
    static void init_TX_serial();
    static void restart_device();
    static void deepsleep(uint32_t sleep_duration_s);
    static void lightsleep(uint16_t sleep_duration_ms);
    static void disable_all_peripherals();
    static void timeout(uint32_t timeout_ms);
    static uint16_t get_remaining_battery_charge();
    static bool set_time_until_deepsleep(uint16_t time_input);
    static inline bool verify_time_input(const uint16_t time);

    static uint32_t deepsleep_time; 
    static uint32_t next_update_time; 

private:
    static RF24Radio* _radio_ptr;
};

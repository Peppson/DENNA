
#include "water_system.h"
#include "capacitor_lite/capacitor_lite.h"


// Members
uint16_t WaterSystem::_sensor_max_value;
uint16_t WaterSystem::_sensor_min_value;


uint8_t WaterSystem::get_water_level() {
    CapacitorLite water_sensor(PIN_WATER_SENSOR_OUT, PIN_WATER_SENSOR_IN);
    constexpr uint8_t samples = 100;

    // Enable ADC and feed the hungry Watchdog
    ADC0.CTRLA |= ADC_ENABLE_bm; 
    WDT_FEED();

    // Sum ADC readings (0-1023)
    uint32_t sum = 0;
    for (size_t i = 0; i < samples; i++) {
        uint16_t level = water_sensor.measure(); 
        sum += constrain(level, 0, 1023);
        delay(1);
    }           
    // Disable ADC
    ADC0.CTRLA &= ~ADC_ENABLE_bm; 

    uint32_t average = sum / samples;
    if (average <= _sensor_min_value) {
        return 0;
    }
    
    return map(average, _sensor_min_value, _sensor_max_value, 0, 100);
}


void WaterSystem::run_water_pump(uint16_t duration_seconds, bool battery_driven) {
    // In case of error...
    if (duration_seconds > WATER_PUMP_RUNTIME_MAX) {
        duration_seconds = 1;
    }

    if (battery_driven) {
        // Ramp up pump with increasing PMW duty cycle
        uint8_t PMW_duty_cycle = 150;
        for (uint8_t i = 0; i < PMW_duty_cycle; i++) {
            analogWrite(PIN_POWER_IO, i);
            delayMicroseconds(100);
        }
        
    // USB driven
    } else {
        digitalWrite(PIN_POWER_IO, HIGH);
    }

    // Monitor water level while pumping
    uint32_t end_time = (duration_seconds * 1000) + millis();
    while (millis() < end_time) {
        WDT_FEED();
        delay(5);

        // Stop if watertank is empty
        if (!get_water_level()) {
            break; 
        } 
    }
    
    // Turn off pump
    digitalWrite(PIN_POWER_IO, LOW);
}

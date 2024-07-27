
#include "water_system.h"
#include "capacitor_lite/capacitor_lite.h"


// Members
uint16_t WaterSystem::_sensor_max_value;
uint16_t WaterSystem::_sensor_min_value;


uint32_t WaterSystem::get_water_level() {
    CapacitorLite water_sensor(PIN_WATER_SENSOR_OUT, PIN_WATER_SENSOR_IN);
    constexpr uint8_t samples = 100;
    WDT_FEED();

    // Enable ADC
    ADC0.CTRLA |= ADC_ENABLE_bm;

    // Set VCC as reference for ADC
    ADC0.CTRLC = TIMEBASE_1US | DEFAULT;

    // Sum readings
    uint32_t sum = 0;
    for (size_t i = 0; i < samples; i++) {
        sum += water_sensor.measure();
    }           

    // Set ADC ref back too 2V5 and disable
    ADC0.CTRLC = TIMEBASE_1US | INTERNAL2V5;
    ADC0.CTRLA &= ~ADC_ENABLE_bm;

    // TODO
    return 42;
    /* 
    uint32_t average = sum / samples;
    if (average <= _sensor_min_value) { return 0; }
    return map(average, _sensor_min_value, _sensor_max_value, 0, 100); 
    */
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
        if (get_water_level() == 0) {
            break; 
        } 
    }
    
    // Turn off pump
    digitalWrite(PIN_POWER_IO, LOW);
}

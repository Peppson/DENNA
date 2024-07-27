
#pragma once
#include "../global_config.h"


class WaterSystem {
public:
    WaterSystem(uint16_t max_value, uint16_t min_value, bool swap_pins = false) {
        if (swap_pins) {
            pinMode(PIN_WATER_SENSOR_OUT, INPUT);
            pinMode(PIN_WATER_SENSOR_IN, OUTPUT);    
        } else {
            pinMode(PIN_WATER_SENSOR_OUT, OUTPUT);
            pinMode(PIN_WATER_SENSOR_IN, INPUT);
        }
        _sensor_max_value = max_value;
        _sensor_min_value = min_value;
    };

    static uint8_t get_water_level();
    static void run_water_pump(uint16_t duration_seconds, bool battery_driven = false);

private:
    static uint16_t _sensor_max_value;
    static uint16_t _sensor_min_value;
};

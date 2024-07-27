
#pragma once
#include "../global_config.h"


class WaterSystem {
public:
    WaterSystem(uint16_t max_value, uint16_t min_value) {
        _sensor_max_value = max_value;
        _sensor_min_value = min_value;
        pinMode(PIN_WATER_SENSOR_IN, INPUT);
        pinMode(PIN_WATER_SENSOR_OUT, INPUT);
    };

    static uint32_t get_water_level();
    static void run_water_pump(uint16_t duration_seconds, bool battery_driven = false);

private:
    static uint16_t _sensor_max_value;
    static uint16_t _sensor_min_value;
};


#pragma once 
#include "../global_config.h"
#include "rf24_radio.h"


namespace util {
    void benchmark(const char* name = nullptr);
    void print_int_hack(const uint32_t& value);
    void calibration();
    void adc_calibration();
    void water_level_calibration();
    void servo_calibration();
    void set_mocca_master_coffee_count();
    void debug_loop();
    //void button_halt_CPU();
}

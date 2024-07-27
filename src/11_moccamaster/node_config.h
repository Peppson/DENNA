
#pragma once
namespace selected_node {

// Starting coffee_count value, if defined will overwrite coffee_count value in memory
//#define COFFEE_COUNT 350UL                                                             

// Node specific globals
static uint16_t coffee_count = 0;                               // Num of coffees made
static bool coffee_state = false;                               // Current state of the machine 

// Node specific configs
constexpr static uint8_t PIN_COFFEE_STATE = PIN_PA6;            // ADC pin
constexpr static uint8_t PIN_SERVO = PIN_PA4;                   // Servo data pin
constexpr static uint8_t SENSOR_ADC_THRESHOLD = 100;            // Current sensor threshold 
constexpr static uint32_t SLEEP_AT_THIS_TIME = 2100UL;          // Sleep at what time? (hhmm)
constexpr static uint32_t SLEEP_DURATION_SECONDS = 8*60*60UL;   // Deepsleep duration in seconds

enum ServoPos : uint16_t {
    ON = 550,
    OFF = 1300,
    MIDDLE_ON = 800,
    MIDDLE_OFF = 950
};
}

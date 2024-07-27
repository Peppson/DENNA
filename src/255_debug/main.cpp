
#include "../global_config.h"
#include "node_program.h"
#include "rf24_radio.h"
#include "hardware.h"
#include "water_system.h"
#include "utility.h"
#include "capacitor_lite/capacitor_lite.h"

//RF24Radio radio;
Hardware hardware(false);
WaterSystem water_system(selected_node::WATER_SENSOR_MIN_VALUE, selected_node::WATER_SENSOR_MAX_VALUE);


void setup() {
    //hardware.begin(&radio);
    hardware.init_TX_serial();
    delay(2000);



    ADC0.CTRLA |= ADC_ENABLE_bm;
    ADC0.CTRLC = TIMEBASE_1US | DEFAULT; 
    CapacitorLite water_sensor(PIN_WATER_SENSOR_OUT, PIN_WATER_SENSOR_IN);

    while (1) {
        uint32_t sum = 0;
        for (size_t i = 0; i < 100; i++) {
            sum += water_sensor.measure();
        }
        log("%i\n", sum / 100); 
        delay(500);
    }
    STOP


    while (1) {
        log("%i\n", water_sensor.measure());
        delay(250);
    }
    STOP


    /* ADC0.CTRLC = TIMEBASE_1US | DEFAULT;
    uint8_t adcRef = ADC0.CTRLC & ADC_REFSEL_gm;
    if (adcRef == VDD) {
        log("1\n");
    } else if (adcRef == EXTERNAL) {
        log("2\n");
    } else if (adcRef == INTERNAL1V024) {
        log("3\n");
    } else if (adcRef == INTERNAL2V048) {
        log("4\n");
    } else if (adcRef == INTERNAL2V5) {
        log("5\n");
    } else if (adcRef == INTERNAL4V096) {
        log("6\n");
    }
    STOP */


    /* while (1) {
        uint16_t level = 0;
        for (size_t i = 0; i < 1000; i++) {
            level = analogRead(PIN_PA6);
        }
        log("%i\n", level);
    } */


    // disaable ADC = 
    // measuring = 

    

    log("GOOOOO!\n");
    while (1) {
        ADC0.CTRLA |= ADC_ENABLE_bm; // enable 

        uint32_t sum = 0;
        for (size_t i = 0; i < 100; i++) {
            uint16_t level = analogRead(PIN_PA6); 
            sum += level;
            delay(1);
        }
        sum = sum / 100;
        log("%i\n", sum);

        ADC0.CTRLA &= ~ADC_ENABLE_bm; // disable
        delay(5000);
   }


    


    digitalWrite(PIN_PA7, LOW);
    uint16_t val = 0;
    while (1) {
        val = hardware.get_remaining_battery_charge();
        log("%i\n", val);
        delay(400);
    }
    
}


void loop() {
    hardware.set_default_pin_states();
} 

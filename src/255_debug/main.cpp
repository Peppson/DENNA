
#include "../global_config.h"
#include "node_program.h"
#include "rf24_radio.h"
#include "hardware.h"
#include "water_system.h"
#include "utility.h"

//RF24Radio radio;
Hardware hardware(true);
//WaterSystem water_system(selected_node::WATER_SENSOR_MIN_VALUE, selected_node::WATER_SENSOR_MAX_VALUE);


void setup() {
    //hardware.begin(&radio);

    hardware.init_TX_serial();
    delay(2000);
    // Disable unused peripherals
    ADC0.CTRLA &= ~ADC_ENABLE_bm;   // ADC 
    TWI0.CTRLA &= ~TWI_ENABLE_bm;   // I2C
    CCL.CTRLA &= ~CCL_ENABLE_bm;    // CCL
    ADC0.CTRLC = TIMEBASE_1US | INTERNAL2V5;

    /* while (1) {
        digitalWrite(PIN_PA7, HIGH);
        delay(2000);
        digitalWrite(PIN_PA7, LOW);
        delay(2000);
    } */

    


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


#include "../global_config.h"
#include "node_program.h"
#include "rf24_radio.h"
#include "hardware.h"
#include "water_system.h"
#include "utility.h"

Hardware hardware(false);
RF24Radio radio;
WaterSystem water_system(selected_node::WATER_SENSOR_MIN_VALUE, selected_node::WATER_SENSOR_MAX_VALUE);


#if CALIBRATION
void setup() { Hardware::begin(&radio); util::calibration(); }
void loop() {}
#else


void setup() {
    hardware.begin(&radio);
    radio.begin();
    selected_node::send_sensor_data(radio, true);
}


void loop() {
    WDT_FEED();

    // Save power by toggling radio transceiver on/off
    radio.power_up();

    // Listen for new radio message
    if (radio.wait_for_message(RF24_ON_TIME)) {
        selected_node::handle_RX_message(radio);
    }

    // Powerdown 
    radio.power_down();
    hardware.lightsleep(RF24_OFF_TIME);

    // Time to send data to master?
    uint32_t cur_time = millis();
    if (cur_time > hardware.next_update_time) {
        selected_node::send_sensor_data(radio, false);
    }

    // Time to sleep?
    if (cur_time > hardware.deepsleep_time) {
        hardware.deepsleep(selected_node::SLEEP_DURATION_SECONDS);
    } 
   
}
#endif   

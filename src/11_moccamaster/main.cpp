
#include "../global_config.h"
#include "node_program.h"
#include "rf24_radio.h"
#include "hardware.h"
#include "memory.h"
#include "utility.h"

Hardware hardware(false);
RF24Radio radio;


#if CALIBRATION || defined(COFFEE_COUNT)
void setup() { Hardware::begin(&radio); util::calibration(); }
void loop() {}
#else


void setup() {
    hardware.begin(&radio);
    radio.begin();

    Memory memory;
    using namespace selected_node;
    coffee_count = memory.get_raid_variable();
    coffee_state = get_mocca_master_state();
    send_sensor_data(radio, true);
}


void loop() {
    WDT_FEED();

    // Save power by toggling radio transceiver on/off
    radio.power_up();

    // Listen for new radio message
    if (radio.wait_for_message(500)) {
        selected_node::handle_RX_message(radio);
    }

    // Powerdown 
    radio.power_down();
    hardware.lightsleep(500);

    // Time to sleep?
    if (millis() > hardware.deepsleep_time) {
        hardware.deepsleep(selected_node::SLEEP_DURATION_SECONDS);
    }

    if (selected_node::check_mocca_master_state()) {
        selected_node::send_sensor_data(radio, false);
    } 
}
#endif

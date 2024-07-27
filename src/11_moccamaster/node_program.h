
#pragma once
#include "../global_config.h"
#include "rf24_radio.h"
#include "hardware.h"
#include "memory.h"

namespace selected_node {


bool get_mocca_master_state() { 
    constexpr uint8_t samples = 100;
    WDT_FEED();
    
    // Enable ADC
    ADC0.CTRLA |= ADC_ENABLE_bm;

    // Sum ADC readings (0-1023)
    uint32_t sum = 0;
    for (uint8_t i = 0; i < samples; i++) { 
        sum += analogRead(PIN_COFFEE_STATE);
    }
    
    // Turn off ADC
    ADC0.CTRLA &= ~ADC_ENABLE_bm; 

    return sum / samples >= SENSOR_ADC_THRESHOLD;                
}


bool check_mocca_master_state() {
    bool cur_state = get_mocca_master_state();

    // Increment coffee_count only if turned on
    if (cur_state && !coffee_state) {
        coffee_state = true;
        coffee_count++;
        Memory memory;
        memory.write_raid_variable(coffee_count);
        return true;
    }

    return false;
}


void move_servo(const ServoPos position) {
    WDT_FEED();
    
    // Set high and low pulses for the SG90 servo (20ms cycles)
    uint16_t high_pulse = position;
    uint16_t low_pulse = 20*1000 - high_pulse;

    // Emulate PWM signal
    for (uint8_t i = 0; i < 20; i++) {
        digitalWrite(PIN_SERVO, HIGH); 
        delayMicroseconds(high_pulse); 
        digitalWrite(PIN_SERVO, LOW); 
        delayMicroseconds(low_pulse); 
    }

    // Move servo back to either ON/OFF middle positions, if moved elsewhere
    // Helps button move freely if pressed by hand
    if (position == ServoPos::ON) { 
        move_servo(ServoPos::MIDDLE_ON); 
    } else if (position == ServoPos::OFF) { 
        move_servo(ServoPos::MIDDLE_OFF);
    }   
}


bool send_data_set_time(RF24Radio& radio) {
    if (radio.send_message(selected_node::coffee_state, selected_node::coffee_count, 0, 0xFFFF) &&
        radio.wait_for_message(3000) &&
        Hardware::set_time_until_deepsleep(radio.message.data_2)) {
        return true;
    }
    
    return false;
}


void send_sensor_data(RF24Radio& radio, const bool request_time) {
    WDT_FEED();

    // Data update
    if (!request_time) {
        radio.send_message(selected_node::coffee_state, selected_node::coffee_count, 0, 0);
        return;
    }

    // Request current time
    if (!send_data_set_time(radio)) {
        Hardware::deepsleep(15*60);
    }

}


void handle_RX_message(RF24Radio& radio) {
    WDT_FEED();

    // Toggle moccamaster power button
    if (radio.message.state) {
        radio.stopListening();
        const ServoPos position = coffee_state ? ServoPos::OFF : ServoPos::ON;
        move_servo(position);
    
    // Commands
    } else switch (radio.message.data_2) {
        case REBOOT: // 0xFFFF
            Hardware::restart_device();
            break;
        case RESEND: // 0xFFFE
            selected_node::send_sensor_data(radio, false);
            break;
        case SLEEP: // 0xFFFD
            Hardware::deepsleep(15*60);
            break;
        default:
            break;
    }

    // Flush buffers in case we recieved another message before "radio.stopListening()"
    if (radio.available()) { 
        radio.flush_rx(); 
    }
}
} // namespace selected_node

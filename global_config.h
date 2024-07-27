
#pragma once 
#include <Arduino.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <stdint.h>


// Dev
#define USB_SERIAL_ENABLED 0                                // Serial TX 
#define DEV_RECOVERY_DELAY 1                                // Used for easier code uploading
#define WATCHDOG_ENABLED 1                                  // Onboard Watchdog
#define DEBUG_LOOP_ENABLED 0                                // Enter debug_loop()
#define ADC_CAL_ENABLED 0                                   // Battery ADC calibration
#define PUMP_CAL_ENABLED 0                                  // Water pump calibration
#define SERVO_CAL_ENABLED 0                                 // Servo calibration

// NRF24L01+ radio
constexpr uint8_t RF24_THIS_ADDRESS = NODE_ID;              // Radio address (env var from platformio.ini)
constexpr uint8_t RF24_MASTER_ADDRESS = 0;                  // Address of master (0)
constexpr uint16_t RF24_ON_TIME = 50;                       // Active
constexpr uint16_t RF24_OFF_TIME = 950;                     // Sleeping                                  
constexpr uint8_t RF24_CHANNEL = 120;                       // Channel 1-126 (check with RF24/examples/scanner/scanner.ino) (120)
constexpr uint8_t RF24_PIPE = 1;                            // Pipe address (1)
#define RF24_OUTPUT (RF24_PA_MAX)                           // Output level: RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX, ERROR
#define RF24_DATARATE (RF24_250KBPS)                        // Datarate: RF24_2MBPS, RF24_1MBPS, RF24_250KBPS

// Misc
constexpr float SLEEP_RTC_INTERVAL_SECONDS = 64.299;        // Calibrated Average time between RTC interrupts while deepsleeping (s)
constexpr uint8_t WATER_PUMP_RUNTIME_MAX = 10;              // Maximum time the waterpump is allowed to run (s) if a flowerpot node
constexpr uint32_t SERIAL_BAUD_RATE = 9600;                 // TX serial rate

// Message commands
enum MsgCommand : uint16_t {
    REBOOT = 0xFFFF,
    RESEND = 0xFFFE,
    SLEEP = 0xFFFD
};

/*
-------------------------- ATtiny824 Pinout --------------------------                                   

                         VCC  1|o   |14 GND                        
             PWM/ADC --- PA4  2|    |13 PA3 --- RF24 SCK          
             PWM/ADC --- PA5  3|    |12 PA2 --- RF24 MISO  
                 ADC --- PA6  4|    |11 PA1 --- RF24 MOSI           
                 ADC --- PA7  5|    |10 PA0 --- UPDI programming          
           Power I/O --- PB3  6|    |9  PB0 --- RF24 CE          
       TX Serial out --- PB2  7|    |8  PB1 --- RF24 CSN  

----------------------------------------------------------------------
*/
#define PIN_CE                  PIN_PB0         // Nrf24l01 Ce pin
#define PIN_CSN                 PIN_PB1         // Nrf24l01 CSN pin
#define PIN_UPDI                PIN_PB0         // UPDI programming
#define PIN_POWER_IO            PIN_PB3         // Controls the "power circuit"
#define PIN_TX_SERAIL           PIN_PB2         // Serial out   
#define PIN_WATER_SENSOR_OUT    PIN_PA4         // Diy capacitive sensor out
#define PIN_WATER_SENSOR_IN     PIN_PA5         // Diy capacitive sensor in

// Watchdog
#if WATCHDOG_ENABLED
    #define WDT_FEED() __asm__ __volatile__ ("wdr") 
    #define WDT_ENABLE() _PROTECTED_WRITE(WDT.CTRLA, WDT_PERIOD_4KCLK_gc)
    #define WDT_DISABLE() wdt_disable()
#else
    #define WDT_FEED()
    #define WDT_ENABLE()
    #define WDT_DISABLE()
#endif

// Logging
#if USB_SERIAL_ENABLED
    #define log(...) Serial.printf(__VA_ARGS__)
    #define STOP log("\n-----  STOP  ----- \n"); while (1) { WDT_FEED(); delay(1000); }
#else
    #define log(...)
    #define STOP
#endif

// Include selected node's "node_config.h" into global space (a bit hacky?)
#if NODE_ID == 1
    #include "src/01_solar_self_watering_plant/node_config.h"
#elif NODE_ID == 2
    #include "src/02_solar_self_watering_plant/node_config.h"
#elif NODE_ID == 6
    #include "src/06_USB_self_watering_plant/node_config.h"
#elif NODE_ID == 7
    #include "src/07_USB_self_watering_plant/node_config.h"
#elif NODE_ID == 11
    #include "src/11_moccamaster/node_config.h"
#elif NODE_ID == 255 // Debug
    #include "src/255_debug/node_config.h"
#else
    #error "Unknown NODE_ID, check platformio.ini"
#endif

// CALIBRATION true if any of the calibration macros are set
#if DEBUG_LOOP_ENABLED || ADC_CAL_ENABLED || PUMP_CAL_ENABLED || SERVO_CAL_ENABLED
    #define CALIBRATION true
#else 
    #define CALIBRATION false
#endif

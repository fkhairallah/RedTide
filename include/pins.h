/*
 *********************************************************************************
                     Hardware pin definition
 *********************************************************************************
 */
#ifndef _PINS_H
#define _PINS_H

// hardware pin definitions
#define pgm_pin 0
//#define blueLED 2      // blue LED light (D1 Mini D4)
#define LED_DATA_PIN_TOP 23     // LED data pin (ESP32 DevKit-V4)
#define LED_DATA_PIN_BOTTOM 22  // LED data pin (ESP32 DevKit-V4)
#define LED_DATA_PIN_TIDE 21    // drive tide H/L indicator
#define NUM_LEDS_TOP 64         // Max # of LEDS
#define NUM_LEDS_BOTTOM 64      // Max # of LEDS

#define COIL_A1 13
#define COIL_A2 14
#define COIL_B3 12
#define COIL_B4 27
#define LIMIT_SWITCH 26

//#define ONE_WIRE_BUS 3 // 1Wire Data in
#define ONE_WIRE_BUS 14

#define LED_BRIGHTNESS 250 // NeoPixel brightness, 0 (min) to 255 (max)

#endif
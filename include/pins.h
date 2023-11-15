/*
 *********************************************************************************
                     Hardware pin definition
 *********************************************************************************
 */
#ifndef _PINS_H
#define _PINS_H

// hardware pin definitions
#define pgm_pin 0               // PGM on reset pin - can be used for control 
#define LED_DATA_PIN_TOP 23     // LED data pin (ESP32 DevKit-V4)
#define LED_DATA_PIN_BOTTOM 22  // LED data pin (ESP32 DevKit-V4)
#define LED_DATA_PIN_TIDE 21    // drive tide H/L indicator
#define MAX_LEDS 64             // Make sure this is larger than any of the NUM LEDS
#define NUM_LEDS_TOP 20         // default # of LEDS (x3 since they are 12V)
#define NUM_LEDS_BOTTOM 20      // default  # of LEDS (x3 since they are 12V)
#define NUM_LEDS_TIDE 3         // number of pixels in tides H/L indicator

#define COIL_A1 13
#define COIL_A2 14
#define COIL_B3 12
#define COIL_B4 27
#define LIMIT_SWITCH 26

//#define ONE_WIRE_BUS 3 // 1Wire Data in
#define ONE_WIRE_BUS 14

#define LED_BRIGHTNESS 250 // NeoPixel brightness, 0 (min) to 255 (max)

#endif
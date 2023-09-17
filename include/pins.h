/*
 *********************************************************************************
                     Hardware pin definition
 *********************************************************************************
 */
#ifndef _PINS_H
#define _PINS_H

// hardware pin definitions
#define pgm_pin 0
#define blueLED 2      // blue LED light (D1 Mini D4)
#define LED_DATA_PIN 4 // LED data pin (D1 Mini D2)

//#define ONE_WIRE_BUS 3 // 1Wire Data in
#define ONE_WIRE_BUS 14

#define NUM_LEDS 40        // Max # of LEDS
#define LED_BRIGHTNESS 250 // NeoPixel brightness, 0 (min) to 255 (max)

#endif
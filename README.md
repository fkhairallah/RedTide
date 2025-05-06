# RedTide

[RED]Tide  is an ESP32-based IOT that implements tide clock using addresseable LED
It is based on RedLED under-cabinet lighting program that was superseeded with WLED


* This program runs undercabinet LED lights, temperature sensor and a tide calculator from a single
 * ESP32

# Version History

## Version 2.5
  change the way homing is done because needle gets stuck on top sometimes.

## Version 2.4
  updated url to reflect NOAA 2025 changes

## Version 2.3
  - added ON/OFF swich for tides

## Version 2.2
  - changed to 2 level dimming
  - added preferences.h to save ledMODE

## Version 2.1
  RC2 -- Many changes:
    - Moved to FastLED library
    - Implemented Stepper Motor to show tide level
    - Implmented color to show tide H/L
    - WS8211 string are now driven via 74HC245
    - Support for upper and lower shelf lights

## Version 2.0
  First iteration of the ESP32
  
## Version 1.0-1.2
  V1.0 was development in the Arduino IDE in 2018
## Version 1.3PIO
  Direct port to Platform.IO and conversion to C++ while still retaining the original JSON5 Library
## Version 1.4
  Updated to use JSON6 library and [RED]Template
## Version 1.5
  Fix to address lack of reliability in LED operation 
  - inability to store # of LED on disk
  - Fridge unit hardware update to include 1000uF and level shifter and in-line resistor on data line -- didn't help
  - Modified logic to turn LEDS on one at a time to reduce current shock
   
# Hardware Notes
 * 
 * 
 * Hardware configuration:
 * 
 *   - Blue LED connected to pin 2
 *   - 1Wire thermocouple connected to pin 3
 *   - WS2812B individually addressable LED pin 4




#ifndef RED_GLOBALS_H
#define RED_GLOBALS_H
#include <dConsole.h>
#include <Ticker.h>

// include Pins
#ifndef _PINS_H
#include <pins.h>
#endif

#define VERSION "V2.0"           // N.B: document changes in README.md
#define MQTT_TOPIC_PREFIX "tide" // prefix for all MQTT topics

// in WIFIConfig
extern char myHostName[];
extern char deviceLocation[];
extern char mqttServer[];
extern char mqttPort[];
extern char numberOfLED[]; // nunber of leds in the strings
extern char NoaaStation[];
void configureWIFI();
void readConfigFromDisk();
void writeConfigToDisk();
void resetConfiguration();

// in console.ino
extern dConsole console;
void setupConsole();
void handleConsole();

#endif
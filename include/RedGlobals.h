

#ifndef RED_GLOBALS_H
#define RED_GLOBALS_H
#include <dConsole.h>
#include <Ticker.h>
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson

// include Pins
#ifndef _PINS_H
#include <pins.h>
#endif

#define VERSION "V0.0"           // N.B: document changes in README.md
#define MQTT_TOPIC_PREFIX "tide" // prefix for all MQTT topics
#define NOAA_BASE_URL "https://api.tidesandcurrents.noaa.gov/api/prod/datagetter?product=predictions&application=NOS.COOPS.TAC.WL&datum=MSL&time_zone=lst_ldt&units=english&interval=hilo&format=json"
#define NOAA_DEFAULT_STATION "8722718"
#define TIDE_UPDATE_INTERVAL 500000L

// in main
extern double minutesToNextTide;
extern double heightOfNextTide;
extern char typeOfNextTide[];

// in WIFIConfig
extern char myHostName[];
extern char deviceLocation[];
extern char mqttServer[];
extern char mqttPort[];
extern char numberOfLED[]; // nunber of leds in the strings
extern char NoaaStation[];
void configureWIFI();
void checkConnection ();
void readConfigFromDisk();
void writeConfigToDisk();
void resetConfiguration();
void configureOTA(char*);

// in console.ino
extern dConsole console;
void setupConsole();
void handleConsole();

#endif
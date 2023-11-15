

#ifndef RED_GLOBALS_H
#define RED_GLOBALS_H
#include <dConsole.h>
#include <Ticker.h>
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson
#include <PubSubClient.h>

#define VERSION "V2.1" // N.B: document changes in README.md

// include Pins
#ifndef _PINS_H
#include <pins.h>
#endif

#define MQTT_TOPIC_PREFIX "tide" // prefix for all MQTT topics

// tide data
#define NOAA_BASE_URL "https://api.tidesandcurrents.noaa.gov/api/prod/datagetter?product=predictions&application=NOS.COOPS.TAC.WL&datum=MSL&time_zone=lst_ldt&units=english&interval=hilo&format=json"
#define NOAA_DEFAULT_STATION "8722718"
#define TIDE_UPDATE_INTERVAL 500000L
#define STEPPER_NUMBER_STEPS 2048 // 28BYJ-48 motor

// in main

// in WIFIConfig
extern char myHostName[];
extern char deviceLocation[];
extern char mqttServer[];
extern char mqttPort[];
extern char mqttUser[];
extern char mqttPwd[];
extern char topLED[]; // nunber of leds in the top string
extern char bottomLED[]; // nunber of leds in the bottom string
extern char NoaaStation[];
extern bool otaInProgress;  // stop doing stuff if we are uploading software

void configureWIFI();
void checkConnection ();
void readConfigFromDisk();
void writeConfigToDisk();
void resetConfiguration();
void configureOTA(char*);

// in lighting
void configureLED();          // update with actual # of LED
void testLED();               // test all LEDs
void setLEDPower(char *mode); // set LED power
void setLEDMode(int mode);    // & mode
void executeLED();
void stripFill(uint32_t color);
void fillList(uint32_t list[], int count);
void fillRainbow();
void setTideMarker(char t);

    // in console.ino
    extern dConsole console;
void setupConsole();
void handleConsole();

// in MQTTConfig
extern bool debugMode;
void configureMQTT();
bool checkMQTTConnection();
void mqttDisconnect();
void mqttCallback(char *topic, byte *payload, unsigned int length);

// in tide.cpp
extern double minutesToNextTide;
extern double heightOfNextTide;
extern char typeOfNextTide[];
extern struct tm nextTideTime;   // time of next tide event
extern struct tm lastTideTime;   // time of last tide event
extern double tideCycleLength;   // length of this tide cycle in minutes
extern double stepsPerMinute;    // depending on tide cycle, number of steps per minute
extern int markerLocation;
void configureTide();
void checkTide();
void getTide(struct tm now);
void homeStepper();
void testStepper();
void step(int n);
void idleStepper();

#endif
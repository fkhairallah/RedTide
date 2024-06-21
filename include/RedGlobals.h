

#ifndef RED_GLOBALS_H
#define RED_GLOBALS_H
#include <Homie.h>

#include <dConsole.h>
#include <Ticker.h>


#define VERSION "V3.0" // N.B: document changes in README.md

// include Pins
#ifndef _PINS_H
#include <pins.h>
#endif

#define MQTT_TOPIC_PREFIX "tide" // prefix for all MQTT topics

// tide data
#define NOAA_BASE_URL "https://api.tidesandcurrents.noaa.gov/api/prod/datagetter?product=predictions&application=NOS.COOPS.TAC.WL&datum=MSL&time_zone=lst_ldt&units=english&interval=hilo&format=json"
#define NOAA_DEFAULT_STATION 8722718
#define TIDE_UPDATE_INTERVAL 900000L    // 500s = 8.3 min, 900 = 15 min
#define STEPPER_NUMBER_STEPS 2048 // 28BYJ-48 motor

// in main
extern bool debugMode;
extern HomieSetting<long> noaaStationNumber;
extern HomieSetting<long> topLED;
extern HomieSetting<long>  bottomLED;
extern HomieSetting<long> ledMode;

// in esp_ota.h
void configureOTA(const char *hostName);
void handleOTA();

// in lighting
void configureLED();           // update with actual # of LED
void testLED();               // test all LEDs
void setLEDPower(char *mode); // set LED power
void setLEDMode(int mode);    // & mode
void executeLED();
void stripFill(uint32_t color);
void fillList(uint32_t list[], int count);
void fillRainbow();
void setTideMarker(char t);
void handleLights();

// in console.ino
extern dConsole console;
void setupConsole();
void handleConsole();


// in tide.cpp
extern bool enableTide;
extern double minutesToNextTide;
extern double heightOfNextTide;
extern char typeOfNextTide[];
extern struct tm nextTideTime;   // time of next tide event
extern struct tm lastTideTime;   // time of last tide event
extern double tideCycleLength;   // length of this tide cycle in minutes
extern double stepsPerMinute;    // depending on tide cycle, number of steps per minute
extern int markerLocation;
void configureTide();
void pauseTideUpdate();
void resumeTideUpdate();
void checkTide();
void getTide(struct tm now);
void homeStepper();
void testStepper();
void step(int n);
void idleStepper();

#endif
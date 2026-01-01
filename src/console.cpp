/**********************************************************************************
 * 
 *    Implements functionality for telnet and serial console
 * 
 *********************************************************************************/
#include <RedGlobals.h>


dConsole console;

void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    console.println("Failed to obtain time");
    return;
  }
  console.print(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  console.printf(" DST=%s\r\n", timeinfo.tm_isdst?"TRUE":"FALSE");
}

/*
 * ********************************************************************************

  ********************  CUSTOMIZABLE SECTION  ***************************

 * ********************************************************************************
*/
#define CUSTOM_COMMANDS "Custom Commands: step n, enable (stepper), home, status, on, off, mode #, topled #, bottomled #, test, noaa"

void executeCustomCommands(char* commandString,char* parameterString)
{
  if (strcmp(commandString, "on") == 0)     setLEDPower((char *)"1");
  if (strcmp(commandString, "off") == 0)    setLEDPower((char *)"0");
  if (strcmp(commandString, "mode") == 0)
  {
    console.print("Color mode: ");
    console.println(parameterString);
    setLEDMode(atoi(parameterString));
  }
  if (strcmp(commandString, "status") == 0)
  {

    printLocalTime();
    console.printf("Next %s tide in %.2f hours (%f feet)\r\n", typeOfNextTide, minutesToNextTide/60, heightOfNextTide);
    console.printf("Marker at %i. cycle is %.1f, %.1f steps per minute\r\n", markerLocation, tideCycleLength, stepsPerMinute);
    console.print(&lastTideTime, "Last tide was @%A, %B %d %Y %H:%M:%S");
    console.println(&nextTideTime, ", next one @%A, %B %d %Y %H:%M:%S\r\n");
    console.println(disableStepper?"Stepper Disabled":"Stepper Enabled");
    //console.printf("Prefs ledMode=%i,%s MQTT=%s #%s\r\n", prefs.getInt("ledMode"), prefs.getString("deviceLocation"), prefs.getString("mqtt_server"), prefs.getString("topLED"));
  }

  if (strcmp(commandString, "enable") == 0)
  {
    disableStepper = false;
    console.println("Stepper Enabled");
  }

  if (strcmp(commandString, "topled") == 0)
  {
    strcpy(topLED, parameterString);
    //writeConfigToDisk();
    savePreferences();
    configureLED();
    console.printf("Number of LEDs changed to %s\r\n", topLED);
  }
  if (strcmp(commandString, "bottomled") == 0)
  {
    strcpy(bottomLED, parameterString);
    //writeConfigToDisk();
    savePreferences();
    configureLED();
    console.printf("Number of LEDs changed to %s\r\n", bottomLED);
  }

  if (strcmp(commandString, "test") == 0)
  {
    testLED();
    
    //testStepper();
    //console.println(digitalRead(LIMIT_SWITCH));
    // struct tm now;
    // if (!getLocalTime(&now))
    // {
    //   console.println("Failed to obtain time");
    //   return;
    // }
    // else
    // {
    //   if (debugMode)
    //     console.print(&now, "%A, %B %d %Y %H:%M:%S, ");
    //     getTide(now);
    // }
  }

  if (strcmp(commandString, "step") == 0)
  {
    step(atoi(parameterString));
    console.printf("Moved %s - now @%i\n", parameterString, markerLocation);
  }
  if (strcmp(commandString, "home") == 0)
  {
    homeStepper();
  }

  if (strcmp(commandString, "noaa") == 0) {
    strcpy(NoaaStation, parameterString);
    //writeConfigToDisk();
    savePreferences();
    console.printf("NOAA station changed to %s\r\n", NoaaStation);
  }
}

/*
 * ********************************************************************************

    ********************  END OF CUSTOMIZABLE SECTION  ***************************

 * ********************************************************************************
*/

void setupConsole()
{
  console.enableSerial(&Serial, true);
  //console.enableTelnet(23);
}


void handleConsole()
{
  // console
  if (console.check())
  {
    executeCustomCommands(console.commandString, console.parameterString);

    if (strcmp(console.commandString, "?") == 0)
    {
      console.print("[RED]");
      console.print(MQTT_TOPIC_PREFIX);
      console.print(" ");
      console.println(VERSION);
      console.printf("Host: %s @", myHostName);
      console.println(WiFi.localIP().toString());
      console.printf("MQTT Server %s, port: %s, %s, LEDs: %s/%s\r\n", mqttServer, mqttPort, deviceLocation,  topLED,bottomLED);
      console.println("Commands: ?, debug, reset (Factory), reboot, quit");
      console.println(CUSTOM_COMMANDS);
    }
    if (strcmp(console.commandString, "debug") == 0)
    {
      debugMode = !debugMode;
      console.print("Debug mode is now ");
      console.println(debugMode);
      prefs.putBool("debugMode", debugMode);
    }
    if (strcmp(console.commandString, "location") == 0)
    {
      strcpy(deviceLocation, console.parameterString);
      //writeConfigToDisk();
      savePreferences();

      console.printf("location changed to %s\r\n", deviceLocation);
      console.println("Change will take effect after next reboot");
    }
    if (strcmp(console.commandString, "mqtt") == 0)
    {
      strcpy(mqttServer, console.parameterString);
      savePreferences();
      //writeConfigToDisk();
      console.print("MQTT server changed to ");
      console.println(mqttServer);
      mqttDisconnect();
    }
    if (strcmp(console.commandString, "reset") == 0)
    {
      console.print("Reseting configuration...");
      resetConfiguration();
      console.println(" Done.");
    }
    if (strcmp(console.commandString, "reboot") == 0)
    {
      console.print("Rebooting...");
      delay(200);
      //reset and try again, or maybe put it to deep sleep
      ESP.restart();
      delay(5000);
    }
    if ((strcmp(console.commandString, "exit") == 0) || (strcmp(console.commandString, "quit") == 0))
    {
      console.print("quiting...");
      console.closeTelnetConnection();
      delay(500);
      console.println("");
    }

    console.print("[RED]> ");
  }
}

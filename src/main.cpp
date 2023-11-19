#include <RedGlobals.h>

void setup()
{
  debugMode = true;
  // setup Console
  setupConsole();

  // configure all LED lines
  configureLED();
  setTideMarker('?');

  configureWIFI(); // configure wifi
  configureMQTT();

  configureTide();

#ifdef TEMP_SENSOR_PRESENT
  // configure thermostat sensor
  tempAccumulator = 0;
  tempNumberOfReading = 0;
  averageTemp = -9999;
  lastTempSend = millis();
  configSensors(_TEMP_SENSOR_PERIOD, &updateTemperature);
#endif
}

void loop()
{
  // This should be the first line in loop();
  checkConnection(); // check WIFI connection & Handle OTA
  handleConsole();   // handle any commands from console

  // Work to be done, but only if we aren't updating the software
  if (!otaInProgress)
  {
#ifdef TEMP_SENSOR_PRESENT
    // service temperature and other sensors
    serviceSensors();
#endif
    checkMQTTConnection(); // check MQTT
    checkTide();           // check tide
    handleLights();        // double check on lighting
    delay(100);
  }
}
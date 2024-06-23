/************************************************************************************

   [RED]Tide

   Configuration parameters are found in .h file

   Tide clock in an old panasonic table-top radio.



 *************************************************************************************/
#include <RedGlobals.h>

// some global definitions...
bool debugMode = false;

/*
 * ********************************************************************************

      Homie setup & loop


 * ********************************************************************************
*/
HomieNode tideNode("tide", "Tide Clock", "switch");
HomieNode ledNode("led", "LED lights", "switch");
HomieNode debugNode("debug", "Debug Mode", "switch");

// custom settings
HomieSetting<long> noaaStationNumber("NoaaStation", "NoaaStation Number");
HomieSetting<long> topLED("topLED", "Number of LED in top shelf");
HomieSetting<long> bottomLED("bottomLED", "Number of LED in bottom shelf");
HomieSetting<long> ledMode("ledMode", "mode of display");

// Code which should run AFTER the WiFi is connected.
void setupHandler()
{

  Homie.getLogger() << "noaaStation: "
                    << noaaStationNumber.get()
                    << endl;

  debugNode.setProperty("on").send(debugMode ? "true" : "false");

  // configure all LED lines
  configureLED();
  // setTideMarker('?');

  configureTide();
}

// Code which should run in normal loop(), after setupHandler() finished.
void loopHandler()
{
  checkTide();     // check tide
  handleLights();  // double check on lighting
  handleConsole(); // handle any commands from console
}

bool changeLights(const HomieRange &range, const String &value)
{
  bool res = false;
  if (debugMode)
    Homie.getLogger() << "lights Handler got " << value << endl;

  if (value == "true")
    res = ledON();
  else
    res = ledOFF();

  ledNode.setProperty("power").send(value);
  return res;
}

bool changeMode(const HomieRange &range, const String &value)
{
  if (debugMode)
    Homie.getLogger() << "mode Handler got " << std::stol(value.c_str()) << endl;

  setLEDMode(std::stol(value.c_str()));
  
  ledNode.setProperty("mode").send(value);

  return true;
}
bool changeTide(const HomieRange &range, const String &value)
{
  Homie.getLogger() << "Tide Handler got " << value << endl;
  // lightNode.setProperty("pause").send(value);

  // Homie.getMqttClient().publish("foo", 1, true,  value.c_str());

  return true;
}

bool changeDebugMode(const HomieRange &range, const String &value)
{
  debugMode = (value == "true");
  debugNode.setProperty("on").send(debugMode ? "true" : "false");
  Homie.getLogger() << "Debug Mode is now: " << debugMode << endl;
  return true;
}

void setup()
{

  // minor hardware setup
  pinMode(blueLED, OUTPUT);

  setupConsole();

  // console.enableSerial(&Serial, true);
  console.print("[RED]Tide ");
  console.println(VERSION);
  Homie_setFirmware("red-tide", VERSION);
  Homie.setLedPin(blueLED, HIGH);
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);

  // define the channels
  ledNode.advertise("power").setName("LED power").setDatatype("boolean").settable(changeLights);
  ledNode.advertise("mode").setName("LED mode").setDatatype("boolean").settable(changeMode);
  tideNode.advertise("pause").setName("Tide").setDatatype("boolean").settable(changeTide);

  debugNode.advertise("on").setName("On").setDatatype("boolean").settable(changeDebugMode);

  Serial << Homie.getConfiguration().deviceId << " Homie Setup...";
  Homie.setup();
  Serial << "Done" << endl;

  configureOTA(Homie.getConfiguration().deviceId);
}

void loop()
{
  Homie.loop();
  handleOTA();
}
/***
 * ********************************************************************************
 * 
 * Implements all lighting functions using FastLED library
 * 
 * 
 * 
 * ********************************************************************************
*/
#include <FastLED.h>
#include <RedGlobals.h>

#define LED_UPDATE_INTERVAL 30000 // LED update interval

bool ledStripON; // Led is on or off
int ledMode = -1;     // mode of display 1 - 2700K, 2 - rainbow,
//CRGB leds[3][MAX_LEDS];
CRGB topShelfLEDS[MAX_LEDS]; 
CRGB bottomShelfLEDS[MAX_LEDS]; 
CRGB tideLEDS[NUM_LEDS_TIDE]; 


long lastLEDUpdate; // hold last time update was sent to LED

/**
 * @brief ocnfigure all 3 led strips using FastLED library
 * 
 * turn off all LEDS and then set the mode to normal
 * 
 */

void configureLED()
{

  console.printf("Configuring top shelf with %s LEDs\r\n", topLED);
  FastLED.addLeds<WS2811, LED_DATA_PIN_TOP, BRG>(topShelfLEDS, atoi(topLED));
  FastLED.addLeds<WS2811, LED_DATA_PIN_BOTTOM, BRG>(bottomShelfLEDS, atoi(bottomLED));
  FastLED.addLeds<WS2811, LED_DATA_PIN_TIDE, GRB>(tideLEDS, NUM_LEDS_TIDE);

  FastLED.clear();
  FastLED.show();
  FastLED.delay(50);

  ledStripON = false;

  ledMode = prefs.getInt("ledMode");
  
}

/***
 * 
 * This is called from loop() repeatadly. It refreshes the lights
 * 
 * This is dones to deals with lights changing randomly after about 30 minutes
 * 
*/
void handleLights() {

  EVERY_N_SECONDS(10) {
    //FastLED.show();
  }

}

/**
 * @brief called from console: used to test various aspects of the LED
 * 
 */
void testLED()
{
  console.printf("ledMode = %i [%i]\r\n", ledMode, prefs.getInt("ledMode"));
  // for (int i = 0; i < NUM_LEDS_TOP; i++)
  // {
  //   topShelfLEDS[i] = CRGB::White;
  //   FastLED.show();
  //   FastLED.delay(1000);
  // }
}

/*
 * ********************************************************************************

   LED Commands

 * ********************************************************************************
*/

/**
 * @brief called in response to MQTT command: turns LEDS on/off
 * 
 * @param mode 
 */

void setLEDPower(char *mode)
{
  //  console.print("setLEDPower=");
  if (((atof(mode) > 0) && (atof(mode) <= 1)) || (strcmp(mode, "1") == 0) || (strcmp(mode, "ON") == 0))
  {
    ledStripON = true;
    console.println("LEDs turned on");
  }
  else
  {
    ledStripON = false;
    console.println("LEDs turned off");
  }

  executeLED();
}

void setLEDMode(int mode)
{
  ledMode = mode;
  prefs.putInt("ledMode", ledMode);  // store in preferences
  executeLED();
}

void executeLED()
{

  if (ledStripON) // if ON determine what to display
  {
    switch (ledMode)
    {
    case 2: // dimmed 
      FastLED.setTemperature(Candle);
      fill_solid(topShelfLEDS, atoi(topLED), CRGB::Grey);
      fill_solid(bottomShelfLEDS, atoi(bottomLED), CRGB::Grey);
      if (debugMode) console.println("DIMMED");
      break;

    case 3: // very dimm
      FastLED.setTemperature(Candle);
      fill_solid(topShelfLEDS, atoi(topLED), CRGB(64,64,64));
      fill_solid(bottomShelfLEDS, atoi(bottomLED), CRGB(64,64,64));
      if (debugMode) console.println("3 - dimmmmmm");
      break;

    case 4: // Navy 
      FastLED.setTemperature(Tungsten100W);  
      fill_solid(topShelfLEDS, atoi(topLED), CRGB::Navy);
      fill_solid(bottomShelfLEDS, atoi(bottomLED), CRGB::Navy);
      if (debugMode) console.println("Navy");
      break;
    case 5: // Rainbow
      FastLED.setTemperature(Tungsten100W);
      fill_rainbow(topShelfLEDS, atoi(topLED), 100);
      fill_rainbow(bottomShelfLEDS, atoi(bottomLED),100);
      if (debugMode) console.println("RAINBOW");
      break;
    case 1:  // 100W tungsten -- Pam's favorite
    default: // full white
      FastLED.setTemperature(Tungsten100W);
      fill_solid(topShelfLEDS, atoi(topLED), CRGB::White);
      fill_solid(bottomShelfLEDS, atoi(bottomLED), CRGB::White);
      if (debugMode) console.println("WHITE");
    }
  }
  else
  {
    fill_solid(topShelfLEDS, atoi(topLED), CRGB::Black);
    fill_solid(bottomShelfLEDS, atoi(bottomLED), CRGB::Black);
    if (debugMode) console.println("OFF");
  }

  FastLED.show();
  FastLED.delay(50);

}



/**
 * @brief sets the tide marker LEDS according to what the next tide is
 *
 * Rising/flowing tide is red
 *
 * Falling/ebbing tide is blue
 *
 * White for everything else (e.g. error., ...)
 *
 *
 */

void setTideMarker(char t)
{

  if (t == 'H')
  {
    fill_solid(tideLEDS, NUM_LEDS_TIDE, CRGB::DarkTurquoise);
    tideLEDS[0] = CRGB::DarkGrey;
  }
  else if (t == 'L')
  {
    fill_solid(tideLEDS, NUM_LEDS_TIDE, CRGB::DarkRed);
    tideLEDS[2] = CRGB::DarkGrey;
  }
  else
  {
    fill_solid(tideLEDS, NUM_LEDS_TIDE, CRGB::Black);
  }
  FastLED.show();
  FastLED.delay(50);
}

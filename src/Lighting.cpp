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
int ledMode;     // mode of display 1 - 2700K, 2 - rainbow,
CRGB leds[3][MAX_LEDS];
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
  FastLED.addLeds<WS2811, LED_DATA_PIN_TOP, BRG>(leds[0], atoi(topLED));
  FastLED.addLeds<WS2811, LED_DATA_PIN_BOTTOM, BRG>(leds[1], atoi(bottomLED));
  FastLED.addLeds<WS2811, LED_DATA_PIN_TIDE, GRB>(leds[2], NUM_LEDS_TIDE);

  FastLED.clear();
  FastLED.show();

  ledStripON = false;
  ledMode = 1;
}

/**
 * @brief called from console: used to test various aspects of the LED
 * 
 */
void testLED()
{

  
  //setLEDMode(5);
  //delay(5000);
  //ledMode = 1
  for (int i = 0; i < NUM_LEDS_TOP; i++)
  {
    leds[0][i] = CRGB::White;
    FastLED.show();
    delay(1000);
  }
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
  executeLED();
}

void executeLED()
{

  if (ledStripON) // if ON determine what to display
  {
    switch (ledMode)
    {
    case 2: // dimmed 50W tungsten
      FastLED.setTemperature(Candle);
      fill_solid(leds[0], NUM_LEDS_TOP, CRGB::Grey);
      fill_solid(leds[1], NUM_LEDS_BOTTOM, CRGB::Grey);
      if (debugMode) console.println("DIMMED");
      break;

    case 3: // navy blue
      fill_solid(leds[0], NUM_LEDS_TOP, CRGB(0, 128, 128));
      fill_solid(leds[1], NUM_LEDS_BOTTOM, CRGB(0, 128, 128));
      if (debugMode) console.println("3 - dimmmmmm");
      break;

    case 4: // Xmas
      // fillList(rgbList, 2);
      console.println("XMAS");
      break;
    case 5: // Rainbow
      fillRainbow();
      // strip.show();
      console.println("RAINBOW");
      break;
    case 1:  // 100W tungsten -- Pam's favorite
    default: // full white
      FastLED.setTemperature(Candle);
      fill_solid(leds[0], NUM_LEDS_TOP, CRGB::White);
      fill_solid(leds[1], NUM_LEDS_BOTTOM, CRGB::White);
      if (debugMode) console.println("ON");
    }
  }
  else
  {
    FastLED.clear();
    console.println("OFF");
  }

  FastLED.show();
  if (debugMode) console.println("LEDs updated");
}



// fill the strip with a sequence of RGB color
void fillRainbow()
{
  fill_rainbow(leds[0], atoi(topLED), 100);
  FastLED.show();
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
    fill_solid(leds[2], NUM_LEDS_TIDE, CRGB::DarkTurquoise);
    leds[2][0] = CRGB::DarkGrey;
  }
  else if (t == 'L')
  {
    fill_solid(leds[2], NUM_LEDS_TIDE, CRGB::DarkRed);
    leds[2][2] = CRGB::DarkGrey;
  }
  else
  {
    fill_solid(leds[2], NUM_LEDS_TIDE, CRGB::White);
  }
  FastLED.show();
}

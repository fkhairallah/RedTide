/*
 * ********************************************************************************

 * ********************************************************************************
*/
#include <FastLED.h>
#include <RedGlobals.h>

#define LED_UPDATE_INTERVAL 30000 // LED update interval

bool ledStripON; // Led is on or off
int ledMode;     // mode of display 1 - 2700K, 2 - rainbow,
CRGB leds[3][30];
long lastLEDUpdate; // hold last time update was sent to LED

void configureLED()
{

  console.printf("Configuring top shelf with %s LEDs\r\n", topLED);
  FastLED.addLeds<WS2811, LED_DATA_PIN_TOP, BRG>(leds[0], atoi(topLED));
  FastLED.addLeds<WS2811, LED_DATA_PIN_BOTTOM, BRG>(leds[1], atoi(bottomLED));
  FastLED.addLeds<WS2811, LED_DATA_PIN_TIDE, BRG>(leds[2], TIDE_INDICATOR_PIXEL_COUNT);

  FastLED.clear();
  FastLED.show();

  ledStripON = false;
  ledMode = 1;
  executeLED();
}

// show a green pattern on the LED strip
void testLED()
{
  setLEDMode(5);
  delay(5000);
  ledMode = 1;

  for (int i = 0; i <= 5; i++)
  {
    setLEDMode(i);
    FastLED.delay(1000);

  }

}

/*
 * ********************************************************************************

   Service an LED command.

 * ********************************************************************************
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
    case 1: // 100W tungsten -- Pam's favorite
      FastLED.setTemperature(Candle);
      FastLED.showColor(CRGB::White);
      console.println("ON");
      break;
    case 2: // dimmed 50W tungsten
      FastLED.setTemperature(Candle);
      FastLED.showColor(CRGB::Grey);
      console.println("DIMMED");
      break;
    case 3: // navy blue
      FastLED.showColor(CRGB(0,128,128));
      console.println("3 - dimmmmmm");
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
    default: // full white
      FastLED.showColor(CRGB::White);
      console.println("Default");
    }
  }
  else
  {
    FastLED.clear();
    FastLED.show();
    console.println("OFF");
  }

  console.println("LEDs updated");
}

// fill entire strip with a single color
void stripFill(uint32_t color)
{

  // for (int i = 0; i < topShelf->numPixels(); i++)
  // {
  //   topShelf->setPixelColor(i, color);
  //   topShelf->show();
  //   //console.printf("LED %d set to %d\r\n", i, color);
  //   delay(50);
  // }
}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait)
{

  // for (int i = 0; i < topShelf->numPixels(); i++)
  // {                                    // For each pixel in strip...
  //   topShelf->setPixelColor(i, color); //  Set pixel's color (in RAM)
  //   delay(wait);                       //  Pause for a moment
  //   topShelf->show();                  //  Update strip to match
  // }
}

// fill the strip with a sequence of RGB color
void fillRainbow()
{
  fill_rainbow(leds[0], atoi(topLED), 100);
  FastLED.show();
}

// fills top shelf with a sequence of color stored in list[]
void fillList(uint32_t list[], int count)
{
  // topShelf->clear();

  // int listCount = 0;
  // for (int i = 0; i < topShelf->numPixels(); i++)
  // { // For each pixel in strip...
  //   topShelf->setPixelColor(i, list[listCount++]);
  //   if (listCount >= count)
  //     listCount = 0;
  //   topShelf->show(); //  Update strip to match
  //   delay(50);
  // }
}

/**
 * @brief sets the tide marker LEDS according to what the next tide is
 * 
 * 
 */

void setTideMarker(char t) {

  if (t == 'H') 
  {
    leds[2][0] = CRGB::Green;
  }
  else
  {
    leds[2][0] = CRGB::Red;
  }
  FastLED.show();
}

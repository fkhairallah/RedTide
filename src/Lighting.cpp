/*
 * ********************************************************************************

 * ********************************************************************************
*/
#include <FastLED.h>
#include <RedGlobals.h>

#define LED_UPDATE_INTERVAL 30000 // LED update interval

bool ledStripON; // Led is on or off
int ledMode;     // mode of display 1 - 2700K, 2 - rainbow,
CRGB leds[30];
long lastLEDUpdate; // hold last time update was sent to LED

// Declare our NeoPixel strip object:
//Adafruit_NeoPixel *topShelf;
// Adafruit_NeoPixel bottomShelf(NUM_LEDS_BOTTOM, LED_DATA_PIN_BOTTOM, NEO_GRB + NEO_KHZ800);
//  Argument 1 = Number of pixels in NeoPixel strip
//  Argument 2 = Arduino pin number (most are valid)
//  Argument 3 = Pixel type flags, add together as needed:
//    NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//    NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//    NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//    NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//    NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
//uint32_t rgbList[] = {topShelf->Color(255, 0, 0), topShelf->Color(0, 255, 0), topShelf->Color(0, 0, 255)};

void configureLED()
{

  console.printf("Configuring top shelf with %s LEDs\r\n", topLED);
  FastLED.addLeds<WS2811,LED_DATA_PIN_TOP, BRG>(leds, atoi(topLED));
  //topShelf = new Adafruit_NeoPixel(atoi(topLED), LED_DATA_PIN_TOP, NEO_GBR + NEO_KHZ800);
  //topShelf->begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  //topShelf->clear(); // Turn OFF all pixels ASAP
  FastLED.clear();
  FastLED.show();
  
  ledStripON = false;
  ledMode = 1;
  executeLED();

}

// show a green pattern on the LED strip
void testLED()
{

  //leds[0] = CRGB::Red; FastLED.show(); delay(1000);
  console.println("Red");
  FastLED.showColor(CRGB::Red); 
  delay(1000);
  console.println("Green");
  FastLED.showColor(CRGB::Green);
  delay(1000);
  console.println("Blue");
  FastLED.showColor(CRGB::Blue);
  delay(1000);
  console.println("White");
  FastLED.showColor(CRGB::AntiqueWhite);
  delay(5000);
  for (int i=0;i<5;i++) {
    FastLED.setTemperature(Candle);
  FastLED.showColor(CRGB::White);
  delay(1000);

FastLED.setTemperature(Tungsten100W);
  FastLED.showColor(CRGB::White);
  delay(1000);
FastLED.setTemperature(Tungsten40W);
  FastLED.showColor(CRGB::White);
  delay(1000);

  }
  

  

  // topShelf->clear();

  // console.println("255,0,0");
  // stripFill(topShelf->gamma32(topShelf->Color(255,0,0)));
  // topShelf->show();
  // delay(1000);

  // for (int i = 0; i < topShelf->numPixels(); i++)
  // {

  //   topShelf->setPixelColor(i, topShelf->Color(255, 0, 0));
  //   topShelf->show();
  //   delay(500);
  // }

  // delay(1000);
  // topShelf->clear();
  // console.println("0,255,0!");

  // for (int i = 0; i < topShelf->numPixels(); i++)
  // {

  //   topShelf->setPixelColor(i, topShelf->Color(0, 255, 0));
  //   topShelf->show();
  //   delay(500);
  // }
  // delay(1000);
  // topShelf->clear();
  // console.println("0,0,255!");

  // for (int i = 0; i < topShelf->numPixels(); i++)
  // {

  //   topShelf->setPixelColor(i, topShelf->Color(0, 0, 255));
  //   topShelf->show();
  //   delay(500);
  // }
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
    case 1:                                    // 100W tungsten -- Pam's favorite
      //stripFill(topShelf->Color(0, 150, 255)); // bGR
      // strip.show();
        FastLED.setTemperature(Candle);

      FastLED.showColor(CRGB::White);
      console.println("ON");
      break;
    case 2: // dimmed 50W tungsten
      //stripFill(topShelf->Color(0, 150 / 2, 255 / 2));
      // strip.show();
      console.println("DIMMED");
      break;
    case 3: // navy blue
      //stripFill(topShelf->Color(0, 128, 128));
      // strip.show();
      console.println("3 - dimmmmmm");
      break;
    case 4: // Xmas
      //fillList(rgbList, 2);
      console.println("XMAS");
      break;
    case 5: // Rainbow
      fillRainbow();
      // strip.show();
      console.println("RAINBOW");
      break;
    default: // full white
      //stripFill(topShelf->gamma32(topShelf->Color(255, 255, 255)));
      // strip.show();
      console.println("Default");
    }
  }
  else
  {
    //stripFill(topShelf->gamma32(topShelf->Color(0, 0, 0)));
    // strip.clear();
    //topShelf->show();
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

  // for (int i = 0; i < topShelf->numPixels(); i++)
  // { // For each pixel in strip...
  //   if ((i % 3) == 0)
  //     topShelf->setPixelColor(i, topShelf->Color(255, 0, 0)); //  Red
  //   if ((i % 3) == 1)
  //     topShelf->setPixelColor(i, topShelf->Color(0, 255, 0)); //  Green
  //   if ((i % 3) == 2)
  //     topShelf->setPixelColor(i, topShelf->Color(0, 0, 255)); //  Blue
  //   topShelf->show();                                         //  Update strip to match
  //   delay(50);
  // }
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

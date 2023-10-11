/*
 * ********************************************************************************

 * ********************************************************************************
*/
#include <Adafruit_NeoPixel.h>
#include <RedGlobals.h>

#define LED_UPDATE_INTERVAL 30000   // LED update interval

bool  ledStripON;     // Led is on or off
int   ledMode;        // mode of display 1 - 2700K, 2 - rainbow,
//CRGB leds[NUM_LEDS];
long lastLEDUpdate;   // hold last time update was sent to LED

// Declare our NeoPixel strip object:
Adafruit_NeoPixel *topShelf;
//Adafruit_NeoPixel bottomShelf(NUM_LEDS_BOTTOM, LED_DATA_PIN_BOTTOM, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
uint32_t rgbList[] = { topShelf->Color(255, 0, 0), topShelf->Color(0, 255, 0), topShelf->Color(0, 0, 255)};

void configureLED()
{

  topShelf = new Adafruit_NeoPixel(atoi(numberOfLED), LED_DATA_PIN_TOP, NEO_GRB + NEO_KHZ800);
  topShelf->begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  topShelf->clear(); // Turn OFF all pixels ASAP
  // topShelf->updateLength(atoi(numberOfLED));
  // topShelf->begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  // topShelf->show();            // Turn OFF all pixels ASAP
  topShelf->setBrightness(LED_BRIGHTNESS); // Set BRIGHTNESS to about 1/5 (max = 255)

  ledStripON = false;
  ledMode = 1;
  executeLED();

  if (debugMode) console.println(String(atoi(numberOfLED)) + " LEDS configured");
}

// show a green pattern on the LED strip
void testLED()
{
  topShelf->clear();

  for (int i = 0; i < atoi(numberOfLED); i++)
  {

    topShelf->setPixelColor(i, topShelf->Color(0, 150, 0));
    topShelf->show();
    console.printf("LED %d set to green\r\n", i);
    delay(50);
  }

}


/*
 * ********************************************************************************

   Service an LED command.

 * ********************************************************************************
*/

void setLEDPower(char* mode)
{
  //  console.print("setLEDPower=");
  if ( ((atof(mode) > 0) && (atof(mode) <= 1) ) || (strcmp(mode, "1") == 0)
      || (strcmp(mode, "ON") == 0) ) {
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
        stripFill(topShelf->gamma32(topShelf->Color(255, 214, 170)));
        //strip.show();
        console.println("ON");
        break;
      case 2: // dimmed 50W tungsten
        stripFill(topShelf->gamma32(topShelf->Color(255 / 2, 202 / 2, 148 / 2)));
        //strip.show();
        console.println("DIMMED");
        break;
      case 3: // navy blue
        stripFill(topShelf->gamma32(topShelf->Color(0, 0, 128)));
        //strip.show();
        console.println("BLUE");
        break;
      case 4: // Xmas
        fillList(rgbList, 2);
        //strip.show();
        console.println("XMAS");
        break;
      case 5: // Rainbow
        fillRainbow();
        //strip.show();
        console.println("RAINBOW");
        break;
      default: // full white
        stripFill(topShelf->gamma32(topShelf->Color(255, 255, 255)));
        //strip.show();
        console.println("Default");
    }
  }
  else
  {
    stripFill(topShelf->gamma32(topShelf->Color(0, 0, 0)));
    //strip.clear();
    topShelf->show();
    console.println("OFF");
  }

  console.println("LEDs updated");

}

// fill entire strip with a single color 
void stripFill(uint32_t color)
{

  for (int i = 0; i < atoi(numberOfLED); i++)
  {
    topShelf->setPixelColor(i, color);
    topShelf->show();
    delay(50);
  }
}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {

  for (int i = 0; i < topShelf->numPixels(); i++) { // For each pixel in strip...
    topShelf->setPixelColor(i, color);         //  Set pixel's color (in RAM)
    delay(wait);//  Pause for a moment
    topShelf->show();                          //  Update strip to match

  }

}

// fill the strip with a sequence of RGB color
void fillRainbow()
{

  for (int i = 0; i < topShelf->numPixels(); i++) { // For each pixel in strip...
    if ( (i % 3) == 0) topShelf->setPixelColor(i, topShelf->Color(255, 0, 0)); //  Red
    if ( (i % 3) == 1) topShelf->setPixelColor(i, topShelf->Color(0, 255, 0)); //  Green
    if ( (i % 3) == 2) topShelf->setPixelColor(i, topShelf->Color(0, 0, 255)); //  Blue
    topShelf->show();                          //  Update strip to match
    delay(50);
  }

}

void fillList(uint32_t list[], int count)
{

  int listCount = 0;
  for (int i = 0; i < topShelf->numPixels(); i++) { // For each pixel in strip...
    topShelf->setPixelColor(i, list[listCount++]);
    if (listCount >= count) listCount = 0;
    topShelf->show();                          //  Update strip to match
    delay(50);
  }

}

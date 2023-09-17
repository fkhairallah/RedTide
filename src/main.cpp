
#include <RedGlobals.h>
#include  <Arduino.h>



void setup()
{
  // setup Console
  setupConsole();

  // configure wifi
  configureWIFI();

}

void loop()
{
  // put your main code here, to run repeatedly:

  handleConsole(); // handle any commands from console
}

#include <RedGlobals.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_TIME_INTERVAL);

void setup()
{
  // setup Console
  setupConsole();

  // configure wifi
  configureWIFI();

  timeClient.begin();
}

void loop()
{
  // put your main code here, to run repeatedly:

  handleConsole(); // handle any commands from console

  timeClient.update();

  Serial.println(timeClient.getFormattedTime());
}
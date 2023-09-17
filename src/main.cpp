
#include <RedGlobals.h>
#include <time.h>
#include <HTTPClient.h>

// Last time tide data was updated
unsigned long lastTideUpdate;


void getTide() {
  char url[256];

    struct tm today;
    if (!getLocalTime(&today))
    {
      console.println("Failed to obtain time");
      return;
    }
    
    // the time library has a trick to do time calculation. You can 
    // modify day, month or year with simple math then call
    // mktime() which will normalize it!!!
    // PS: structures can be copied in c++ with a simple assignment
    struct tm tomorrow = today;
    tomorrow.tm_mday += 1;  
    mktime(&tomorrow);

  // now we generate the GET request and get the JSON response from NOAA

  // form the URL
    sprintf(url, "%s&station=%s&begin_date=%02d/%02d/%d&end_date=%02d/%02d/%d",
            NOAA_BASE_URL, NoaaStation,
            today.tm_mon + 1, today.tm_mday, today.tm_year + 1900,
            tomorrow.tm_mon + 1, tomorrow.tm_mday, tomorrow.tm_year + 1900);

    //console.println(url);

    HTTPClient http;
    String payload;   // response possibly containing JSON
    
    http.begin(url);
    int httpResponseCode = http.GET();

  // check for valid server response
    if (httpResponseCode > 0)
    {
      // valid -- get the JSON object
      //console.print("HTTP Response code: ");
      //console.println(httpResponseCode);
      payload = http.getString();
      console.println(payload);
    }
    else
    {
      console.print("Error code: ");
      console.println(httpResponseCode);
    }
    // Free resources
    http.end();

    DynamicJsonDocument json(4096);
    auto deserializeError = deserializeJson(json, payload);
    serializeJson(json, Serial);
    if (!deserializeError)
    {
      if (json.containsKey("predictions")) {
        JsonArray tides = json["predictions"];
        console.print(tides.size());
        console.println(" predictions");
        for (int i=0; i< tides.size(); i++) {
          console.print(i);console.print(": ");
          String t = tides[i]["t"];
          String hl = tides[i]["type"];
          console.print(t);
          console.print(" ");
          console.println(hl);
        }

      }
    }
}

void setup()
{
  // setup Console
  setupConsole();

  // configure wifi
  configureWIFI();

  // init and get current time
  configTime(0,0,"pool.ntp.org");

  // timezone strings here:https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
  setenv("TZ", "EST5EDT, M3 .2.0, M11 .1.0", 1); //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();

  // last Time tide was updated
  lastTideUpdate = millis() + 2000 - TIDE_UPDATE_INTERVAL;

  delay(1000);
}

void loop()
{
  // put your main code here, to run repeatedly:

  handleConsole(); // handle any commands from console

  // check on time every so often
  if ((millis() - lastTideUpdate) >  TIDE_UPDATE_INTERVAL)
  {

      getTide();
      lastTideUpdate = millis();
  }
}
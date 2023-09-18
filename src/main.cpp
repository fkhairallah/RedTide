
#include <RedGlobals.h>
#include <time.h>
#include <HTTPClient.h>

// Last time tide data was updated
unsigned long lastTideUpdate;
double minutesToNextTide;
double heightOfNextTide;
char typeOfNextTide[32];

void getTide()
{
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

  // console.println(url);

  HTTPClient http;
  String payload; // response possibly containing JSON

  http.begin(url);
  int httpResponseCode = http.GET();

  // check for valid server response
  if (httpResponseCode > 0)
  {
    // valid -- get the JSON object
    // console.print("HTTP Response code: ");
    // console.println(httpResponseCode);
    payload = http.getString();
  }
  else
  {
    console.print("Error getting NOAA data. code: ");
    console.println(httpResponseCode);
    http.end();
    return;
  }
  // Free resources
  http.end();

  DynamicJsonDocument json(4096);
  auto deserializeError = deserializeJson(json, payload);
  // serializeJson(json, Serial);
  if (!deserializeError)
  {
    if (json.containsKey("predictions"))
    {
      JsonArray tides = json["predictions"];
      console.printf("\r\nNOAA sent %d preditions\r\n",tides.size());
      for (int i = 0; i < tides.size(); i++)
      {
        // get next tide time into a workable format
        char t[32], v[32], type[32];
        strcpy(t, tides[i]["t"]);
        strcpy(v, tides[i]["v"]);
        strcpy(type, tides[i]["type"]);
        Serial.printf("%d: %s %s %s\r\n", i, t, v, type);

        int year, month, day, hour, min;
        sscanf(t, "%d-%d-%d  %d:%d", &year, &month, &day, &hour, &min);

        struct tm tideTime;
        tideTime.tm_year = year - 1900;
        tideTime.tm_mon = month - 1;
        tideTime.tm_mday = day;
        tideTime.tm_hour = hour;
        tideTime.tm_min = min;

        // see if this tide is in the future
        time_t tidett = mktime(&tideTime);
        time_t nowtt = mktime(&today);

        if (tidett > nowtt)
        {
          minutesToNextTide = difftime(tidett, nowtt) / 60;
          strcpy(typeOfNextTide, type);
          heightOfNextTide = atof(v);
         console.printf("Next %s tide in %.2f minutes (%f feet)\r\n\r\n", typeOfNextTide, minutesToNextTide, heightOfNextTide);
        break;
        }
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

  delay(2000);

  // init and get current time
  configTime(0, 0, "pool.ntp.org");

  // timezone strings here:https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
  setenv("TZ", "EST5EDT, M3 .2.0, M11 .1.0", 1); //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();

  // last Time tide was updated
  lastTideUpdate = millis() + 2000 - TIDE_UPDATE_INTERVAL;

}

void loop()
{
  // put your main code here, to run repeatedly:
  checkConnection(); // check WIFI connection

  handleConsole(); // handle any commands from console

  // check on time every so often
  if ((millis() - lastTideUpdate) > TIDE_UPDATE_INTERVAL)
  {

    getTide();
    lastTideUpdate = millis();
  }
}
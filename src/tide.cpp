#include <RedGlobals.h>
#include <time.h>
#include <HTTPClient.h>
#include <Stepper.h>

#define COIL_A1 13
#define COIL_A2 14
#define COIL_B3 12
#define COIL_B4 27

// Last time tide data was updated
unsigned long lastTideUpdate;
double minutesToNextTide;
double heightOfNextTide;
char typeOfNextTide[32];

// stepper motor configuration for 28BYJ-48 motor
Stepper stepper(STEPPER_NUMBER_STEPS, COIL_A1, COIL_A2, COIL_B3, COIL_B4);

void testStepper()
{
    stepper.setSpeed(10);
    // step one revolution  in one direction:
    console.println("1 rev clockwise");
    stepper.step(STEPPER_NUMBER_STEPS);
    delay(500);

    // step one revolution in the other direction:
    console.println("1 rev counterclockwise");
    stepper.step(-1000);
    delay(500);

    parkStepper();
}
void parkStepper() {

    digitalWrite(COIL_A1, 0);
    digitalWrite(COIL_A2, 0);
    digitalWrite(COIL_B3, 0);
    digitalWrite(COIL_B4, 0);
}

/*
 * ********************************************************************************

 use the NOAA web service to get the tide forecast for 'NoaaStation'. This routine
 will then parse the json and determine the next tide event storing the values in
 minutesToNextTide, heightOfNextTide and typeOfNextTide (H/L)

 * ********************************************************************************
*/

void getTide()
{
    char url[256];

    if (debugMode) console.println("Getting tide information....");

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
            if (debugMode) console.printf("\r\nNOAA sent %d preditions\r\n", tides.size());
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
                    if (debugMode) console.printf("Next %s tide in %.2f minutes (%f feet)\r\n\r\n", typeOfNextTide, minutesToNextTide, heightOfNextTide);
                    break;
                }
            }
        }
    }
}
/*
 * ********************************************************************************

 initialize the tides configuration. Get current time and set the timezone.
 This is currently hard coded and can be moved into the configuraton later

 * ********************************************************************************
*/

void configureTide()
{
    // init and get current time
    configTime(0, 0, "pool.ntp.org");

    // timezone strings here:https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
    setenv("TZ", "EST5EDT, M3 .2.0, M11 .1.0", 1); //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
    tzset();

    // last Time tide was updated
    lastTideUpdate = millis() + 2000 - TIDE_UPDATE_INTERVAL;
}
/*
 * ********************************************************************************

 periodically check on tides and update according to the configered interval

 * ********************************************************************************
*/

void checkTide()
{

    // check on time every so often
    if ((millis() - lastTideUpdate) > TIDE_UPDATE_INTERVAL)
    {

        getTide();
        lastTideUpdate = millis();
    }
}
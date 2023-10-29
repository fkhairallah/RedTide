#include <RedGlobals.h>
#include <time.h>
#include <HTTPClient.h>
#include <Stepper.h>


// range of dial -- keep motor operating within
#define STEPPER_MAX_RANGE 4500L // measured

// total ide period is 12 hours and 50 minutes so half of that (high-low) is 385 minutes
// Divide range by 385 to get the steps per minutes
#define STEPS_PER_MINUTE 11.688

// Last time tide data was updated
unsigned long lastTideUpdate;
double minutesToNextTide;
double heightOfNextTide;
char typeOfNextTide[32];
bool disableStepper = true;

int markerLocation;

// stepper motor configuration for 28BYJ-48 motor
Stepper stepper(STEPPER_NUMBER_STEPS, COIL_A1, COIL_A2, COIL_B3, COIL_B4);

// this routine 
// it configures the limit switch
/**
 * @brief This routine configures all the necessary items for proper operation of the stepper motor:
 * 1. Configures the limit switch
 * 2. home the marker so we start from a know position
 *
 */
void homeStepper() {


    console.println("Homing Stepper...");
    markerLocation = 0;

    pinMode(LIMIT_SWITCH, INPUT_PULLUP);

    stepper.setSpeed(10);

    // if we're pegged -- move down then try hominh
    if (digitalRead(LIMIT_SWITCH) == 0) {
        stepper.step(-1000);
        console.println("Limit hit--- descending a bit...");
    }

    while(digitalRead(LIMIT_SWITCH)!= 0) {
        stepper.step(50);
        markerLocation += 50;
        if (markerLocation >= STEPPER_MAX_RANGE)
        {
            disableStepper = true;
            idleStepper();
            console.println("Cannot Home marker. Too many steps...");
            return;
        }
    }

    // we've successfully arrived home.
    idleStepper();
    disableStepper = false;
    markerLocation = STEPPER_MAX_RANGE;
    console.println("Homing successfull!");

}



// take steps while making sure that we stay within range
// n can be positive (move dial up) or negative (move dial down)
void step(int n)
{
    if (disableStepper) return;

    // make sure we do not exceed the limit
    if ( (markerLocation + n) < 0 ) n = -markerLocation;
    if ( (markerLocation + n) >= STEPPER_MAX_RANGE) n = STEPPER_MAX_RANGE - markerLocation;

    // set soeed takse the steps then idle the motor
    stepper.setSpeed(10);
    stepper.step(n);
    idleStepper();

    // update the marker location
    markerLocation += n;
}


void idleStepper() {

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
    else
    {
        console.printf("Time now is %d:%d ", today.tm_hour, today.tm_min);
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
                tideTime.tm_isdst = today.tm_isdst; // make sure DST matches

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
    // zero the stepper motor by returning it to the home position
    homeStepper();

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
    double markerNewLocation;
    int stepsToTake; 

    // check on time every so often
    if ((millis() - lastTideUpdate) > TIDE_UPDATE_INTERVAL)
    {

        getTide();
        lastTideUpdate = millis();

        if (typeOfNextTide == "L")
            markerNewLocation =  (385 - minutesToNextTide) * STEPS_PER_MINUTE;
        else
            markerNewLocation = minutesToNextTide * STEPS_PER_MINUTE;

        stepsToTake = (int)markerNewLocation - markerLocation;

        console.printf("%0.1f minutes left - moving to %0.1f by %i\n", minutesToNextTide, markerNewLocation, stepsToTake);

        step(stepsToTake);
    }
}
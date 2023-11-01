#include <RedGlobals.h>
#include <time.h>
#include <HTTPClient.h>
#include <Stepper.h>


// range of dial -- keep motor operating within
#define STEPPER_MAX_RANGE 4250L // measured

// total ide period is 12 hours and 50 minutes so half of that (high-low) is 385 minutes
// Divide range by 385 to get the steps per minutes
#define STEPS_PER_MINUTE 11.0

// Last time tide data was updated
unsigned long lastMarkerUpdate;   // interval between marker update
struct tm nextTideTime;         // time of next tide event
struct tm lastTideTime;         // time of last tide event
double tideCycleLength;         // length of this tide cycle in minutes
double stepsPerMinute;          // depending on tide cycle, number of steps per minute
double minutesToNextTide;       // minutes left in this cycle
double heightOfNextTide;        // height above MLLW in feet
char typeOfNextTide[32];        // H/L
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

    if (debugMode)
        console.print("Homing Stepper...");
    markerLocation = 0;

    pinMode(LIMIT_SWITCH, INPUT_PULLUP);

    stepper.setSpeed(10);

    // if we're pegged -- move down then try hominh
    if (digitalRead(LIMIT_SWITCH) == 0) {
        stepper.step(-1000);
        if (debugMode)
            console.print("Limit hit--- descending a bit...");
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
    if (debugMode)
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

// seems the library keeps some pins on causing current within the winding of the stepper motor
// it causes heating. Turn all pins off to save power.
void idleStepper() {

    digitalWrite(COIL_A1, 0);
    digitalWrite(COIL_A2, 0);
    digitalWrite(COIL_B3, 0);
    digitalWrite(COIL_B4, 0);
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

    // give 2s delay before updating tide to allow system to fully configure
    lastMarkerUpdate = millis() + 2000 - TIDE_UPDATE_INTERVAL;

    struct tm today;
    if (!getLocalTime(&today))
    {
        console.println("configureTide() Failed to obtain time");
        return;
    }
    else
    {
        getTide(today);
    }

}
/*
 * ********************************************************************************

 use the NOAA web service to get the tide forecast for 'NoaaStation'. This routine
 will then parse the json and determine the next tide event storing the values in
 minutesToNextTide, heightOfNextTide and typeOfNextTide (H/L)

 * ********************************************************************************
*/

void getTide(struct tm now)
{
    // turn the time to time_t so we can do math on it
    time_t nowtt = mktime(&now);

    char url[256];

    if (debugMode) console.print("\nGetting tide information....");

    // the time library has a trick to do time calculation. You can
    // modify day, month or year with simple math (+ - * /)
    // then call mktime() which will normalize it!!!
    // PS: structures can be copied in c++ with a simple assignment
    struct tm tomorrow = now;
    tomorrow.tm_mday += 1;
    mktime(&tomorrow);

    // now we generate the GET request and get the JSON response from NOAA

    // form the URL
    sprintf(url, "%s&station=%s&begin_date=%02d/%02d/%d&end_date=%02d/%02d/%d",
            NOAA_BASE_URL, NoaaStation,
            now.tm_mon + 1, now.tm_mday, now.tm_year + 1900,
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
            if (debugMode) console.printf("NOAA sent %d preditions\n", tides.size());
            for (int i = 0; i < tides.size(); i++)
            {
                // get next tide time into a workable format
                char t[32], v[32], type[32];
                strcpy(t, tides[i]["t"]);
                strcpy(v, tides[i]["v"]);
                strcpy(type, tides[i]["type"]);
                if (debugMode) console.printf("%d: %s %s %s  ", i, t, v, type);

                int year, month, day, hour, min;
                sscanf(t, "%d-%d-%d  %d:%d", &year, &month, &day, &hour, &min);
                if (debugMode) console.printf("parsed %i/%i/%i %i:%i\n", year, month, day, hour, min);

                nextTideTime.tm_year = year - 1900;
                nextTideTime.tm_mon = month - 1;
                nextTideTime.tm_mday = day;
                nextTideTime.tm_hour = hour;
                nextTideTime.tm_min = min;
                nextTideTime.tm_isdst = now.tm_isdst; // make sure DST matches

                // see if this tide is in the future
                time_t tidett = mktime(&nextTideTime);

                if (tidett > nowtt)
                {
                    minutesToNextTide = difftime(tidett, nowtt) / 60;
                    strcpy(typeOfNextTide, type);
                    heightOfNextTide = atof(v);
                    tideCycleLength = difftime(tidett, mktime(&lastTideTime) ) / 60;
                    stepsPerMinute = STEPPER_MAX_RANGE / tideCycleLength;
                    if (debugMode) console.printf("Next %s tide in %.2f minutes (%.1f feet) spm %.1f\n", typeOfNextTide, minutesToNextTide, heightOfNextTide, stepsPerMinute);
                    if (debugMode) console.print("Last Tide was at ");
                    if (debugMode) console.print(&lastTideTime, "%A, %B %d %Y %H:%M:%S");
                    if (debugMode) console.print(" next at ");
                    if (debugMode) console.println(&nextTideTime, "%A, %B %d %Y %H:%M:%S\n");
                    break;
                }
                else 
                {
                    lastTideTime = nextTideTime;
                    if (lastTideTime.tm_hour != nextTideTime.tm_hour) console.println("***TIME ASSIGNMENT FAILED***");
                }
            }
        }
    }
}

/*
 * ********************************************************************************

 periodically check on tides and move marker to correct location

 * ********************************************************************************
*/
void checkTide()
{
    double markerNewLocation;
    int stepsToTake; 

    // check on time every so often
    if ((millis() - lastMarkerUpdate) > TIDE_UPDATE_INTERVAL)
    {
        struct tm today;
        if (!getLocalTime(&today))
        {
            console.println("Failed to obtain time");
            return;
        }
        else
        {
            if (debugMode) console.print(&today, "%A, %B %d %Y %H:%M:%S, ");
        }

        // calculate minutes left
        minutesToNextTide = difftime(mktime(&nextTideTime) , mktime(&today)) / 60;

        // get tide if we are past the nextTideTime
        if (minutesToNextTide < 0)
        {
            getTide(today);

        }

        if (typeOfNextTide[0] == 'H')
            markerNewLocation =  (385 - minutesToNextTide) * stepsPerMinute;
        else
            markerNewLocation = minutesToNextTide * stepsPerMinute;

        stepsToTake = (int)markerNewLocation - markerLocation;

        if (debugMode) console.printf("%0.1f minutes left - moving to %0.1f by %i\n", minutesToNextTide, markerNewLocation, stepsToTake);

        step(stepsToTake);

        lastMarkerUpdate = millis();
    }
}
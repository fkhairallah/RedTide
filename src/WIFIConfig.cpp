#include <RedGlobals.h>
#include <FS.h> //this needs to be first, or it all crashes and burns...
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include <SPIFFS.h>
#include <ArduinoOTA.h>

bool otaInProgress; // flags if OTA is in progress

// configuration parameters
// Hostname, AP name & MQTT clientID
char myHostName[64] = "RedTide";
char deviceLocation[64] = "NEW";

// define your default values here, if there are different values in config.json, they are overwritten.
// length should be max size + 1
char mqttServer[40] = "carbon.local";
char mqttPort[6] = "443";
char numberOfLED[16] = "6";
char NoaaStation[16] = NOAA_DEFAULT_STATION;

// The extra parameters to be configured (can be either global or just in the setup)
// After connecting, parameter.getValue() will get you the configured value
// id/name placeholder/prompt default length
WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqttServer, 40);
WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqttPort, 5);
WiFiManagerParameter custom_number_led("numberOfLED", "Number of LEDs", numberOfLED, 16);
WiFiManagerParameter custom_noaa_station("NoaaStation", "Noaa Station", NoaaStation, 16);

// flag for saving data
bool shouldSaveConfig = false;

// callback notifying us of the need to save config
void saveConfigCallback()
{
    Serial.println("Should save config");
    shouldSaveConfig = true;
}
/*
 * ********************************************************************************

 * ********************************************************************************
*/

void configureWIFI()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial.println();

    // clean FS, for testing
    // SPIFFS.format();

    // read configuration from FS json
    readConfigFromDisk();



    // WiFiManager
    // Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;

    // set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);


    // add all your parameters here
    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_number_led);
    wifiManager.addParameter(&custom_noaa_station);

    // reset settings - for testing
    // wifiManager.resetSettings();

    // set minimu quality of signal so it ignores AP's under that quality
    // defaults to 8%
    wifiManager.setMinimumSignalQuality();

    // sets timeout until configuration portal gets turned off
    // useful to make it all retry or go to sleep
    // in seconds
    // wifiManager.setTimeout(120);

    // fetches ssid and pass and tries to connect
    // if it does not connect it starts an access point with the specified name
    // here  "AutoConnectAP"
    // and goes into a blocking loop awaiting configuration
    if (!wifiManager.autoConnect("AutoConnectAP", "password"))
    {
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        // reset and try again, or maybe put it to deep sleep
        ESP.restart();
        delay(5000);
    }

    // if you get here you have connected to the WiFi
    Serial.println("connected...yeey! Enabling telent:)");
    console.enableTelnet(23);

    // // read updated parameters
    // strcpy(mqttServer, custom_mqtt_server.getValue());
    // strcpy(mqttPort, custom_mqtt_port.getValue());
    // strcpy(api_token, custom_api_token.getValue());
    
     // and OTA
    configureOTA(myHostName);

    // save the custom parameters to FS
    if (shouldSaveConfig)
        writeConfigToDisk();

    console.printf("local ip: "); console.println(WiFi.localIP());

}
/*
 * ********************************************************************************
 * This routine will check the Wifi status, and reset the ESP is unable to connect
 *
 * If all is well it initiates mDNS service & OTA process
 *
 * ********************************************************************************
 */

void checkConnection()
{
    if (WiFi.status() != WL_CONNECTED) // reconnect wifi
    {
        console.println("Not connected to WIFI.. give it ~10 seconds.");
        delay(1000);
        // if (secondsWithoutWIFI++ > 30)
        // {
        //   ESP.reset();
        //   delay(5000);
        // }
    }

    //MDNS.update(); // and refresh mDNS

    // handle OTA -- if in progress stop talking to the heat pump and console so as not to disturb the upload
    // THIS NEEDS TO BE THE FIRST ITEM IN LOOP
    ArduinoOTA.handle();
}
/*
 * ********************************************************************************

 read device configuration from config.json in the file system

 * ********************************************************************************
*/
void readConfigFromDisk()
{
    Serial.println("mounting FS...");

    if (SPIFFS.begin())
    {
        Serial.println("mounted file system");
        if (SPIFFS.exists("/config.json"))
        {
            // file exists, reading and loading
            Serial.println("reading config file");
            File configFile = SPIFFS.open("/config.json", "r");
            if (configFile)
            {
                Serial.println("opened config file");
                size_t size = configFile.size();
                // Allocate a buffer to store contents of the file.
                std::unique_ptr<char[]> buf(new char[size]);

                configFile.readBytes(buf.get(), size);
                DynamicJsonDocument json(1024);
                auto deserializeError = deserializeJson(json, buf.get());
                serializeJson(json, Serial);
                if (!deserializeError)
                {
                    if (json.containsKey("mqtt_server"))  strcpy(mqttServer, json["mqtt_server"]);
                    if (json.containsKey("mqtt_port"))    strcpy(mqttPort, json["mqtt_port"]);
                    if (json.containsKey("numberOfLED"))  strcpy(numberOfLED, json["numberOfLED"]);
                    if (json.containsKey("NoaaStation"))  strcpy(NoaaStation, json["NoaaStation"]);
                }
                else
                {
                    Serial.println("failed to load json config");
                }
            }
        }
    }
    else
    {
        Serial.println("failed to mount FS");
    }
    // end read
}
/*
 * ********************************************************************************

 updates the configuration in config.json in the file system

 * ********************************************************************************
*/
void writeConfigToDisk()
{
    Serial.println("saving config");
#if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
    DynamicJsonDocument json(1024);
#else
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();
#endif
    json["mqtt_server"] = mqttServer;
    json["mqtt_port"] = mqttPort;
    json["numberOfLED"] = numberOfLED;
    json["NoaaStation"] = NoaaStation;


    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile)
    {
        Serial.println("failed to open config file for writing");
    }

#if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
    serializeJson(json, Serial);
    serializeJson(json, configFile);
#else
    json.printTo(Serial);
    json.printTo(configFile);
#endif
    configFile.close();
    // end save
}

/*
 * ********************************************************************************

  Reset all configuration paramters.

 * ********************************************************************************
*/
void resetConfiguration()
{
    WiFiManager wm;
    wm.resetSettings();
}


/*
 * ********************************************************************************
 * 
 * ********************************************************************************
 */
void configureOTA(char *hostName)
{
  // configure OTA
  otaInProgress = false; // used to stop other activities during OTA update

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(hostName);

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    otaInProgress = true;
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      type = "sketch";
    }
    else
    { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    console.println("Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
    otaInProgress = false;
    console.println("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    char buffer[100];
    sprintf(buffer, "Progress: %u%%\r", (progress / (total / 100)));
    console.println(buffer);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    otaInProgress = false;
    char buffer[100];
    sprintf(buffer, "Error[%u]: ", error);
    console.println(buffer);
    if (error == OTA_AUTH_ERROR)
    {
      console.println("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR)
    {
      console.println("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR)
    {
      console.println("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR)
    {
      console.println("Receive Failed");
    }
    else if (error == OTA_END_ERROR)
    {
      console.println("End Failed");
    }
  });

  ArduinoOTA.begin();
}

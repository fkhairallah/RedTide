/**********************************************************************************
 *
 * Configure the MQTT server by:
 *     - create all the topic using prefix/location/subtopic
 *     - configure MQTT server and port and setup callback routine
 *     - attempt a connection and log to debug topic if success
 * 
 *********************************************************************************/
#include <RedGlobals.h>


WiFiClient espClient;
PubSubClient mqtt_client(espClient);

// mqtt client settings
char clientName[64];
char mqtt_topic[64] = "xtide/default";                                         //contains current settings
char mqtt_temperature_topic[64] = "tide/default/temperature";                 //temperature
#ifdef DISPLAY_PRESENT
char mqtt_requiredTemperature_topic[64] = "led/default/requiredTemperature"; //listens for commands
#endif
char mqtt_debug_topic[64] = "tide/default/debug";                             //debug messages
char mqtt_debug_set_topic[64] = "tide/default/debug/set";                     //enable/disable debug messages

char mqtt_led_command[64] = "tide/LOCATION/set"; // contains LED control commands
char mqtt_led_mode[64] = "tide/LOCATION/mode";   // contains LED mode

char mqtt_tide_command[64] = "tide/LOCATION/tide";  // start and stop tide indicator

int secondsWithoutMQTT;

// MQTT Settings
// debug mode, when true, will send all packets received from the heatpump to topic mqtt_debug_topic
// this can also be set by sending "on" to mqtt_debug_set_topic
bool debugMode = false;
bool retain = true; //change to false to disable mqtt retain


/*
 * ********************************************************************************

  ********************  CUSTOMIZABLE SECTION  ***************************

 * ********************************************************************************
*/

// configure all topics based on function & location
void configureTopics() 
{
  sprintf(clientName, "%s-%s", myHostName, deviceLocation);
  sprintf(mqtt_topic, "%s/%s", MQTT_TOPIC_PREFIX, deviceLocation);
  sprintf(mqtt_debug_topic, "%s/debug", mqtt_topic);
  sprintf(mqtt_debug_set_topic, "%s/debug/set", mqtt_topic);
#ifdef DISPLAY_PRESENT
  sprintf(mqtt_requiredTemperature_topic, "%s/requiredTemperature", mqtt_topic);
#endif
#ifdef TEMP_SENSOR_PRESENT
  sprintf(mqtt_temperature_topic, "%s/temperature", mqtt_topic);
#endif
  sprintf(mqtt_led_command, "%s/set", mqtt_topic);
  sprintf(mqtt_led_mode, "%s/mode", mqtt_topic);
  sprintf(mqtt_tide_command, "%s/tide", mqtt_topic);
}

// this is called when a connection is established with the server
// it subscribe to all define and needed topics
void subscribeToTopics()
{
  
#ifdef DISPLAY_PRESENT
  mqtt_client.subscribe(mqtt_requiredTemperature_topic);
#endif
  mqtt_client.subscribe(mqtt_led_command);
  mqtt_client.subscribe(mqtt_led_mode);

  // tide on/off topic, start with ON
  mqtt_client.subscribe(mqtt_tide_command);
  mqtt_client.publish(mqtt_tide_command, "ON");

  // debug topic (?)
  mqtt_client.subscribe(mqtt_debug_set_topic);
}

// This routine is called when an MQTT message is received 
// it handles any needed functionality and return TRUE
// returns FALSE if the topic is outside the scope of this function
bool processMQTTcommand(char* topic, char* message)
{
  if (strcmp(topic, mqtt_led_command) == 0)
  {
    setLEDPower(message);

    // publish state back to main topic
    mqtt_client.publish(mqtt_topic, message);
    return true;
  }
  
  if (strcmp(topic, mqtt_led_mode) == 0)
  {
    setLEDMode(atoi(message));

    // publish state back to main topic
    mqtt_client.publish(mqtt_topic, message);
    return true;
  }

  // turn tide on/off
  if (strcmp(topic, mqtt_tide_command) == 0)
  {
    if (strcmp(message,"OFF") == 0) 
      pauseTideUpdate();
    else
      resumeTideUpdate();
    return true;
  }

#ifdef DISPLAY_PRESENT
  if (strcmp(topic, mqtt_requiredTemperature_topic) == 0)
  {
    int temp = atoi(message);
    if ((temp > 45) && (temp < 100))
    {
      requiredTemperature = temp;
      displayRequiredTemperature(temp);
      writeConfigToDisk(); // save it
    }
    return true;
  }
  else
  
#endif
  return false;
}

#ifdef TEMP_SENSOR_PRESENT
// Publish the temperature to the MQTT server
void mqttPublishTemperature(char* tempStr)
{
  mqtt_client.publish(mqtt_temperature_topic, tempStr);
}
#endif

/*
 * ********************************************************************************

    ********************  END OF CUSTOMIZABLE SECTION  ***************************

 * ********************************************************************************
*/

/*
 * ********************************************************************************

   This routine handles all MQTT callbacks and processes the commands sent to hp
   1. it extracts the topic & message
   2. it routes it to processMQTTcommand to handle app-specific actions
   3. Handles house keeping command such as turning debug on/off

 * ********************************************************************************
*/
void mqttCallback(char *topic, byte *payload, unsigned int length)
{

  // Copy payload into message buffer
  char message[length + 1];
  for (unsigned int i = 0; i < length; i++)
  {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';

  // try processing main commands
  if (!processMQTTcommand(topic, message))
  {
    if (strcmp(topic, mqtt_debug_set_topic) == 0)
    {
      if (strcmp(message, "on") == 0)
      {
        debugMode = true;
        mqtt_client.publish(mqtt_debug_topic, "debug mode enabled");
      }
      else if (strcmp(message, "off") == 0)
      {
        debugMode = false;
        mqtt_client.publish(mqtt_debug_topic, "debug mode disabled");
      }
    }
    else
    {
      mqtt_client.publish(mqtt_debug_topic, strcat((char*)"wrong mqtt topic: ", topic));
    }
  }
}


/*
 * ********************************************************************************

   Configure the MQTT server by:
    - create all the topic using prefix/location/subtopic
    - configure MQTT server and port and setup callback routine
    - attempt a connection and log to debug topic if success

 * ********************************************************************************
*/

void configureMQTT()
{

  // configure the topics using location
  configureTopics();

  // configure mqtt connection
  mqtt_client.setServer(mqttServer, atoi(mqttPort));
  mqtt_client.setCallback(mqttCallback);

  console.print("MQTT Server :'");
  console.print(mqttServer);
  console.print("' Port: ");
  console.print(String(atoi(mqttPort)));
  console.print(" Topic set to: '");
  console.print(mqtt_topic);
  console.println("'");

}

/*********************************************************************************

   attemps a connection to the MQTT server. if it fails increment secondsWithoutMQTT
   and return.

   This code relies on an existing Wifi connection which checked and dealt with
   elsewhere in the code

 **********************************************************************************/

bool checkMQTTConnection() {

  // loop through the client
  mqtt_client.loop();

  if (!mqtt_client.connected()) 
  {
    console.printf("Status %i - ", mqtt_client.state());

    // Attempt to connect
    if (mqtt_client.connect(clientName))
    {
      subscribeToTopics();
      console.printf("Connected to MQTT as %s\r\n", clientName);
      char str[128];
      sprintf(str, "%s %s: @[%s] IP:%i.%i.%i.%i", clientName, VERSION, deviceLocation, WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
      mqtt_client.publish(mqtt_debug_topic, str, true);
      secondsWithoutMQTT = 0;
      return true;
    }
    else
    {
      delay(500);
      secondsWithoutMQTT++;
      return false;
    }
  }
  return true;
}

/*********************************************************************************
 * 
 * Disconnect from the MQTT server. This is done when server name or port have
 * changed and we need to reconnect
 * 
 *********************************************************************************/

void mqttDisconnect()
{
  if (mqtt_client.connected()) mqtt_client.disconnect();
}


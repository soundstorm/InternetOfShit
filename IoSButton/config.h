/*
 * Configure your settings here
 */

IPAddress MQTT_SERVER(10, 0, 0, 1); // IP of your Broker
//const char* MQTT_SERVER = "mqtt.arduino-hannover.de"; // Or FQDN
const char* MQTT_USER = NULL;
const char* MQTT_PASSWORD = NULL;

const char* WIFI_SSID = "ssid";
const char* WIFI_PASSWORD = "password";

/*
 * By default the button is recognized as binary sensor
 * If you want to have it as switch, use the following
 */
//#define HASS_SWITCH

/*
 * Configure your settings here
 */

IPAddress MQTT_SERVER(10, 0, 0, 1); // IP of your Broker
//const char* MQTT_SERVER = "mqtt.arduino-hannover.de"; // Or FQDN
const char* MQTT_USER = NULL;
const char* MQTT_PASSWORD = NULL;

const char* WIFI_SSID = "ssid";
const char* WIFI_PASSWORD = "password";

// You may need to adjust this to fit your resistor tolerance exactly
const double ADC_TO_VOLTAGE = 0.004324894515;

// If you don't want the block alert to trigger and shut down WiFi
//#define GRACEFUL_SHUTDOWN

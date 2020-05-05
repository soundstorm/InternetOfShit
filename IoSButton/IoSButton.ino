/*
 * This example acts as simple toggle switch for Home Assistant
 * Configuration is automatically done via MQTT auto discovery
 * Just configure your MQTT and WiFi settings and you're good to go
 * Please do not change the entity-id as it's used as base for toggling
 * You can change the name via Home Assistant if you like
 * 
 * Change config.h to match your network parameters
 * 
 * Use ESP8285 as target
 */

#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include "config.h"

const char SW_VERSION[] = "1.4";

const uint8_t SHDN_PIN = 16;
const uint8_t LED_PIN  = 2;

const uint8_t NUM_BUTTONS = 3;
boolean button[NUM_BUTTONS];
Adafruit_NeoPixel led = Adafruit_NeoPixel(1, LED_PIN, NEO_RGB + NEO_KHZ800);

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
	digitalWrite(SHDN_PIN, HIGH);
	pinMode(SHDN_PIN, OUTPUT);
	pinMode(4, INPUT_PULLUP);
	pinMode(14, INPUT_PULLUP);
	pinMode(13, INPUT_PULLUP);
	pinMode(12, INPUT_PULLUP);
	button[0] = digitalRead(4);
	button[1] = digitalRead(14);
	// To compensate different ESP-M3 Pinouts we check for both pins
	button[2] = digitalRead(13) & digitalRead(12);
	led.begin();
	Serial.begin(115200);

	WiFi.mode(WIFI_STA);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

	uint32_t con_start = millis();
	int8_t f = 0;
	Serial.println("Connecting...");
	while (con_start + 10000 > millis() && WiFi.status() != WL_CONNECTED) {
		led.setPixelColor(0, abs(f), abs(f), 0);
		led.show();
		delay(10);
		f++;
	}
	
	if (WiFi.status() != WL_CONNECTED) {
		Serial.println("Failed");
		led.setPixelColor(0, 128, 0, 0);
		led.show();
		delay(2000);
		Serial.println("Shutting down.");
		digitalWrite(SHDN_PIN, LOW);
		delay(50);
	}
	client.setServer(MQTT_SERVER, 1883);
	
	Serial.println("Connecting MQTT...");
	if (!client.connect((String("IoS-Button-") + ESP.getChipId()).c_str(), MQTT_USER, MQTT_PASSWORD)) {
		Serial.println("MQTT Failed");
		led.setPixelColor(0, 128, 0, 128);
		led.show();
		delay(2000);
		Serial.println("Shutting down.");
		pinMode(SHDN_PIN, INPUT);
		delay(50);
		ESP.restart();
	}
	client.loop();
	
	client.setServer(MQTT_SERVER, 1883);
	String dev = String("\"dev\":{\"ids\":[\"") + ESP.getChipId() + "\"],\"cns\":[[\"mac\",\"" + WiFi.macAddress() + "\"]],\"name\":\"IoS-Button\",\"mf\":\"HannIO\",\"mdl\":\"Internet of Shit Button\",\"sw\":\"" + SW_VERSION + "\"}";
	String msg;
#ifdef HASS_SWITCH
	String config_topic_base = "homeassistant/switch/ios-button-";
#else //HASS_SWITCH
	String config_topic_base = "homeassistant/binary_sensor/ios-button-";
#endif //HASS_SWITCH
	for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
#ifdef HASS_SWITCH
		msg = String("{\"name\":\"IoS-Button ") + ESP.getChipId() + " " + i + "\"," + dev + ",\"uniq_id\":\"ios-button-" + ESP.getChipId() + "-" + i + "\",\"stat_t\":\"~\",\"cmd_t\":\"~\",\"t\":\"ios-button/" + ESP.getChipId() + "/" + i + "\",\"val_tpl\":\"{%if is_state('switch.ios_button_" + ESP.getChipId() + "_" + i + "',\\\"on\\\")-%}OFF{%-else-%}ON{%-endif%}\"}";
#else  //HASS_SWITCH
		msg = String("{\"name\":\"IoS-Button ") + ESP.getChipId() + " " + i + "\"," + dev + ",\"uniq_id\":\"ios-button-" + ESP.getChipId() + "-" + i + "\",\"stat_t\":\"ios-button/" + ESP.getChipId() + "/" + i + "\",\"val_tpl\":\"{%if is_state('binary_sensor.ios_button_" + ESP.getChipId() + "_" + i + "',\\\"on\\\")-%}OFF{%-else-%}ON{%-endif%}\"}";
#endif //HASS_SWITCH
		client.beginPublish((config_topic_base + ESP.getChipId() + "-" + i + "/config").c_str(), msg.length(), true);
		client.print(msg.c_str());
		client.endPublish();
	}
	led.setPixelColor(0, 0x00AA00);
	led.show();
}

void blink(uint8_t n, uint32_t c) {
	for (uint8_t i = 0; i < n; i++) {
		led.setPixelColor(0, c);
		led.show();
		delay(150);
		led.setPixelColor(0, 0);
		led.show();
		delay(150);
	}
}

void loop() {
	// You may also combine button presses
	if (button[0]) { // IO4
		client.publish((String("ios-button/") + ESP.getChipId() + "/0").c_str(), "TOGGLE");
		blink(1, 0x8800);
	} else if (button[1]) { // IO14
		client.publish((String("ios-button/") + ESP.getChipId() + "/1").c_str(), "TOGGLE");
		blink(2, 0x8800);
	} else if (button[2]) { // IO13/12
		client.publish((String("ios-button/") + ESP.getChipId() + "/2").c_str(), "TOGGLE");
		blink(3, 0x8800);
	} else { // No closed contact recognized
	}
	for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
		Serial.print("Button ");
		Serial.print(i+1);
		Serial.print(": ");
		Serial.println(button[i]);
	}
	client.disconnect();
	WiFi.mode(WIFI_OFF);
	if (digitalRead(4) || digitalRead(14) || (digitalRead(13) & digitalRead(12))) {
		// Not yet all contacts open again
		blink(1, 0x000088);
		led.setPixelColor(0, 0x010000); // Set dim red to show "I'm alive" but not drawing too much current
		led.show();
	}
	pinMode(SHDN_PIN, INPUT);
	while (true) delay(10);
}

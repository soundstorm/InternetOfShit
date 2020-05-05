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

const char SW_VERSION[] = "1.5";

const uint8_t SHDN_PIN = 16;
const uint8_t LED_PIN  = 2;

const uint8_t NUM_BUTTONS = 3;
boolean button[NUM_BUTTONS];
#ifdef LONGPRESS_ENABLED
boolean button_long[NUM_BUTTONS];
#endif //LONGPRESS_ENABLED
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
#ifdef LONGPRESS_ENABLED
	/*
	 * Set button_long to true:
	 * As long as button[i] == 1 and button_long[i] == 1 the button is not released
	 * If timer is over LONGPRESS_DURATION and still not released, button[i] is set to 0
	 * If button is released before the timer check, action is handled by interrupt
	 * So:
	 * button[i] == 1, button_long[i] == 1: Currently pressed but long press duration not over yet
	 * button[i] == 1, button_long[i] == 0: Short button press detected
	 * button[i] == 0, button_long[i] == 1: Long press detected
	 * button[i] == 0, button_long[i] == 0: Not pressed at all
	 */
	button_long[0] = button[0];
	button_long[1] = button[1];
	button_long[2] = button[2];
	if (button[0]) {
		attachInterrupt(digitalPinToInterrupt(4), lp_int_0, FALLING);
	}
	if (button[1]) {
		attachInterrupt(digitalPinToInterrupt(14), lp_int_1, FALLING);
	}
	if (button[2]) {
		attachInterrupt(digitalPinToInterrupt(13), lp_int_2, FALLING);
		attachInterrupt(digitalPinToInterrupt(12), lp_int_2, FALLING);
	}
#endif //LONGPRESS_ENABLED
	
	led.begin();
	Serial.begin(115200);
	Serial.println();
	Serial.println(millis());

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
#ifdef LONGPRESS_ENABLED
	String msg_long;
#endif //LONGPRESS_ENABLED
#ifdef HASS_SWITCH
	String config_topic_base = "homeassistant/switch/ios-button-";
#else //HASS_SWITCH
	String config_topic_base = "homeassistant/binary_sensor/ios-button-";
#endif //HASS_SWITCH
	for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
#ifdef HASS_SWITCH
		msg = String("{\"name\":\"IoS-Button ") + ESP.getChipId() + " " + i + "\"," + dev + ",\"uniq_id\":\"ios-button-" + ESP.getChipId() + "-" + i + "\",\"opt\":1,\"stat_t\":\"~\",\"cmd_t\":\"~cmd\",\"~\":\"ios-button/" + ESP.getChipId() + "/" + i + "\",\"val_tpl\":\"{%if is_state('switch.ios_button_" + ESP.getChipId() + "_" + i + "',\\\"on\\\")-%}OFF{%-else-%}ON{%-endif%}\"}";
#ifdef LONGPRESS_ENABLED
		msg_long = String("{\"name\":\"IoS-Button ") + ESP.getChipId() + " " + i + " Long\"," + dev + ",\"uniq_id\":\"ios-button-" + ESP.getChipId() + "-" + i + "-long\",\"opt\":1,\"stat_t\":\"~\",\"cmd_t\":\"~cmd\",\"~\":\"ios-button/" + ESP.getChipId() + "/" + i + "-long\",\"val_tpl\":\"{%if is_state('switch.ios_button_" + ESP.getChipId() + "_" + i + "_long',\\\"on\\\")-%}OFF{%-else-%}ON{%-endif%}\"}";
#endif //LONGPRESS_ENABLED
#else  //HASS_SWITCH
		msg = String("{\"name\":\"IoS-Button ") + ESP.getChipId() + " " + i + "\"," + dev + ",\"uniq_id\":\"ios-button-" + ESP.getChipId() + "-" + i + "\",\"stat_t\":\"ios-button/" + ESP.getChipId() + "/" + i + "\",\"val_tpl\":\"{%if is_state('binary_sensor.ios_button_" + ESP.getChipId() + "_" + i + "',\\\"on\\\")-%}OFF{%-else-%}ON{%-endif%}\"}";
#ifdef LONGPRESS_ENABLED
		msg_long = String("{\"name\":\"IoS-Button ") + ESP.getChipId() + " " + i + " Long\"," + dev + ",\"uniq_id\":\"ios-button-" + ESP.getChipId() + "-" + i + "-long\",\"stat_t\":\"ios-button/" + ESP.getChipId() + "/" + i + "-long\",\"val_tpl\":\"{%if is_state('binary_sensor.ios_button_" + ESP.getChipId() + "_" + i + "_long',\\\"on\\\")-%}OFF{%-else-%}ON{%-endif%}\"}";
#endif //LONGPRESS_ENABLED
#endif //HASS_SWITCH
		client.beginPublish((config_topic_base + ESP.getChipId() + "-" + i + "/config").c_str(), msg.length(), true);
		client.print(msg.c_str());
		client.endPublish();
#ifdef LONGPRESS_ENABLED
		client.beginPublish((config_topic_base + ESP.getChipId() + "-" + i + "-long/config").c_str(), msg_long.length(), true);
		client.print(msg_long.c_str());
		client.endPublish();
#endif //LONGPRESS_ENABLED
	}
	led.setPixelColor(0, 0x00AA00);
	led.show();
#ifdef LONGPRESS_ENABLED
	// Wait for buttons released or timeout
	while (millis() <= LONGPRESS_DURATION) {
		if (!(button[0] & button_long[0]) & !(button[1] & button_long[1]) & !(button[2] & button_long[2])) {
			// buttons released and recognized by ISR
			break;
		}
		yield();
	}
	// Deactivate interrupts if not done already
	detachInterrupt(digitalPinToInterrupt(4));
	detachInterrupt(digitalPinToInterrupt(14));
	detachInterrupt(digitalPinToInterrupt(13));
	detachInterrupt(digitalPinToInterrupt(12));
	// If button not released yet, set longpress (by unsetting button[i])
	for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
		if (button[i] & button_long[i]) {
			button[i] = 0;
		}
	}
#endif //LONGPRESS_ENABLED
}

#ifdef LONGPRESS_ENABLED
ICACHE_RAM_ATTR void lp_int_0(void) {
	detachInterrupt(digitalPinToInterrupt(4));
	if (millis() > LONGPRESS_DURATION) {
		button[0] = 0;
	} else {
		button_long[0] = 0;
	}
}
ICACHE_RAM_ATTR void lp_int_1(void) {
	detachInterrupt(digitalPinToInterrupt(14));
	if (millis() > LONGPRESS_DURATION) {
		button[1] = 0;
	} else {
		button_long[1] = 0;
	}
}
ICACHE_RAM_ATTR void lp_int_2(void) {
	detachInterrupt(digitalPinToInterrupt(13));
	detachInterrupt(digitalPinToInterrupt(12));
	if (millis() > LONGPRESS_DURATION) {
		button[2] = 0;
	} else {
		button_long[2] = 0;
	}
}
#endif //LONGPRESS_ENABLED

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
	if (button_long[0]) { // IO4
		client.publish((String("ios-button/") + ESP.getChipId() + "/0-long").c_str(), "TOGGLE");
		blink(1, 0x88);
	} else if (button_long[1]) { // IO14
		client.publish((String("ios-button/") + ESP.getChipId() + "/1-long").c_str(), "TOGGLE");
		blink(2, 0x88);
	} else if (button_long[2]) { // IO13/12
		client.publish((String("ios-button/") + ESP.getChipId() + "/2-long").c_str(), "TOGGLE");
		blink(3, 0x88);
	} else if (button[0]) { // IO4
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
	client.loop();
	for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
		Serial.print("Button ");
		Serial.print(i+1);
		Serial.print(": ");
#ifdef LONGPRESS_ENABLED
		Serial.println(button[i] ? "pressed" : button_long[i] ? "hold" : "off");
#else  //LONGPRESS_ENABLED
		Serial.println(button[i] ? "pressed" : "off");
#endif //LONGPRESS_ENABLED
	}
	// Wait until published
	delay(1000);
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

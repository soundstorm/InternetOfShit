/*
 * This example acts as mailbox notifier for Home Assistant
 * Configuration is automatically done via MQTT auto discovery
 * Just configure your MQTT and WiFi settings and you're good to go
 * Switch for occupancy on (new mail) on IO4
 * Switch for occupancy off (postbox empty or at least looked inside ;-) on IO5
 * Switches must only close if action should be triggered.
 * So generally normally connected tactile switches are useful.
 */

#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

IPAddress MQTT_SERVER(10, 0, 0, 1); // IP of your Broker
//const char* MQTT_SERVER = "mqtt.arduino-hannover.de"; // Or FQDN
const char* MQTT_USER = NULL;
const char* MQTT_PASSWORD = NULL;

const char* WIFI_SSID = "ssid";
const char* WIFI_PASSWORD = "password";

// You may need to adjust this to fit your resistor tolerance exactly
const double ADC_TO_VOLTAGE = 0.004324894515;

const char SW_VERSION[] = "1.5";

const uint8_t SHDN_PIN = 16;
const uint8_t LED_PIN  = 2;

boolean button[5];
Adafruit_NeoPixel led = Adafruit_NeoPixel(1, LED_PIN, NEO_RGB + NEO_KHZ800);

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
	digitalWrite(SHDN_PIN, HIGH);
	pinMode(SHDN_PIN, OUTPUT);
	// For use with Internet of Shit Socket
	// Connect mail detection switch to IO4 (new mail)
	// Connect mail removal detection switch to IO5 (empty)
	pinMode(4, INPUT_PULLUP);
	pinMode(5, INPUT_PULLUP);
	pinMode(12, INPUT_PULLUP);
	pinMode(13, INPUT_PULLUP);
	pinMode(14, INPUT_PULLUP);
	button[0] = digitalRead(4);
	button[1] = digitalRead(5);
	button[2] = digitalRead(13);
	button[3] = digitalRead(12);
	button[4] = digitalRead(14);
	led.begin();
	Serial.begin(115200);
  
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
	// Send "no problem" detected when disconnecting as last will
	if (!client.connect("IoS-Briefkasten", MQTT_USER, MQTT_PASSWORD, "briefkasten/b", 0, (bool)true, "OFF")) {
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
	String dev = String("\"dev\":{\"ids\":[\"") + ESP.getChipId() + "\"],\"cns\":[[\"mac\",\"" + WiFi.macAddress() + "\"]],\"name\":\"Briefkasten\",\"mf\":\"HannIO\",\"mdl\":\"Internet of Shit Socket\",\"sw\":\"+ SW_VERSION +\"}";
	String msg = String("{\"name\":\"Briefkasten\",") + dev + ",\"uniq_id\":\"ios-briefkasten\",\"stat_t\":\"briefkasten\",\"dev_cla\":\"occupancy\"}";
	client.beginPublish("homeassistant/binary_sensor/briefkasten/config", msg.length(), true);
	client.print(msg.c_str());
	client.endPublish();
	// You may disable this sensor in Home Assistant as the attributes of ios-briefkasten-bat also reflect this value
	msg = String("{\"name\":\"Briefkasten Spannung\",") + dev + ",\"uniq_id\":\"ios-briefkasten-volt\",\"stat_t\":\"briefkasten/v\",\"unit_of_meas\":\"V\",\"ic\":\"mdi:flash\"}";
	client.beginPublish("homeassistant/sensor/briefkasten_voltage/config", msg.length(), true);
	client.print(msg.c_str());
	client.endPublish();
	msg = String("{\"name\":\"Briefkasten Akku\",") + dev + ",\"uniq_id\":\"ios-briefkasten-bat\",\"stat_t\":\"briefkasten/a\",\"unit_of_meas\":\"%\",\"json_attr_t\":\"briefkasten/att\",\"dev_cla\":\"battery\"}";
	client.beginPublish("homeassistant/sensor/briefkasten_voltage/config", msg.length(), true);
	client.print(msg.c_str());
	client.endPublish();
	msg = String("{\"name\":\"Briefkasten Geblockt\",") + dev + ",\"uniq_id\":\"ios-briefkasten-block\",\"stat_t\":\"briefkasten/b\",\"dev_cla\":\"problem\"}";
	client.beginPublish("homeassistant/binary_sensor/briefkasten_block/config", msg.length(), true);
	client.print(msg.c_str());
	client.endPublish();
	publishBattery();
}

void publishBattery(void) {
	uint16_t adc = analogRead(0);
	double voltage = analogRead(0) * ADC_TO_VOLTAGE;
	uint8_t percent = 0;
	if (voltage > 4.15) percent = 100; else
	if (voltage > 4.11) percent =  95; else
	if (voltage > 4.08) percent =  90; else
	if (voltage > 4.02) percent =  85; else
	if (voltage > 3.98) percent =  80; else
	if (voltage > 3.95) percent =  75; else
	if (voltage > 3.91) percent =  70; else
	if (voltage > 3.87) percent =  65; else
	if (voltage > 3.85) percent =  60; else
	if (voltage > 3.84) percent =  55; else
	if (voltage > 3.82) percent =  50; else
	if (voltage > 3.80) percent =  45; else
	if (voltage > 3.79) percent =  40; else
	if (voltage > 3.77) percent =  35; else
	if (voltage > 3.75) percent =  30; else
	if (voltage > 3.73) percent =  25; else
	if (voltage > 3.71) percent =  20; else
	if (voltage > 3.69) percent =  15; else
	if (voltage > 3.61) percent =  10; else
	if (voltage > 3.55) percent =   5;
	char att[28];
	sprintf(att, "{\"ADC\":%d,\"Voltage\":%1.2f}", adc, voltage);
	client.publish("briefkasten/att", att, (bool)true);
	client.publish("briefkasten/v", String(voltage).c_str(), (bool)true);
	client.publish("briefkasten/a", String(percent).c_str(), (bool)true);
	
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
	if (button[0]) { // IO4
		client.publish("briefkasten", "ON", (bool)true);
		blink(1, 0x8800);
	} else if (button[1]) { // IO5
		client.publish("briefkasten", "OFF", (bool)true);
		blink(2, 0x8800);
	} else if (button[2]) { // IO13
		blink(3, 0x88);
	} else if (button[3]) { // IO12
		blink(4, 0x88);
	} else if (button[4]) { // IO14
		blink(5, 0x88);
	} else { // No closed contact recognized
	}
	/*
	 * Graceful shutdown, if you don't want to report problem/voltage while any contact is still closed
	 * Use client.disconnect() but then comment client.publish / publishBattery at the end
	 */
	//client.disconnect();
	for (uint8_t i = 0; i < 5; i++) {
		Serial.print("Button ");
		Serial.print(i+1);
		Serial.print(": ");
		Serial.println(button[i]);
	}
	if (!digitalRead(4) || !digitalRead(5) || !digitalRead(12) || !digitalRead(13) || !digitalRead(14)) {
		// Not yet all contacts open again
		blink(1, 0x000088);
		led.setPixelColor(0, 0x010000); // Set dim red to show "I'm alive" but not drawing too much current
		led.show();
	}
	delay(200);
	// Turn off (at least try it)
	pinMode(SHDN_PIN, INPUT);
	while (true) { // As long as any contact is closed report battery voltage every 60s
		delay(60000);
		// Something keeps us awake, activate "problem" sensor
		// Stays on until we disconnect and send our last will
		client.publish("briefkasten/b", "ON", (bool)true);
		publishBattery();
	}
	// Deepsleep doesn't work as the ESP is drawing too less current then to shutdown the system
}

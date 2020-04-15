#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>

const uint8_t SHDN_PIN = 16;
const uint8_t LED_PIN  = 2;

boolean button[3];
Adafruit_NeoPixel led = Adafruit_NeoPixel(1, LED_PIN, NEO_RGB + NEO_KHZ800);
bool otaRunning = false;

WiFiClient espClient;
PubSubClient client(espClient);
IPAddress mqtt_server(10, 0, 0, 1);

void setup() {
	digitalWrite(SHDN_PIN, HIGH);
	pinMode(SHDN_PIN, OUTPUT);
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
  
	WiFi.begin("SSID", "password");
  
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
	client.setServer(mqtt_server, 1883);
	
	Serial.println("Connecting MQTT...");
	if (!client.connect("Briefkasten")) {
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
	client.publish("homeassistant/binary_sensor/briefkasten/config", "{\"name\":\"Briefkasten\",\"stat_t\":\"briefkasten\",\"pl_on\":\"ON\",\"pl_off\":\"OFF\"}", (bool)true);
	client.publish("homeassistant/sensor/briefkasten_voltage/config", "{\"name\":\"Briefkasten Spannung\",\"stat_t\":\"briefkasten/v\",\"unit_of_meas\":\"V\"}", (bool)true);
	client.publish("briefkasten/v", String(analogRead(0) * 0.004324894515).c_str(), (bool)true);
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
	if (button[0]) {
		client.publish("briefkasten", "ON", (bool)true);
		blink(1, 0x8800);
	} else if (button[1]) {
		client.publish("briefkasten", "OFF", (bool)true);
		blink(2, 0x8800);
	} //...
	// client.disconnect(); //graceful shutdown
	for (uint8_t i = 0; i < 5; i++) {
		Serial.print("Button ");
		Serial.print(i+1);
		Serial.print(": ");
		Serial.println(button[i]);
	}
	if (!digitalRead(4) || !digitalRead(5) || !digitalRead(12) || !digitalRead(13) || !digitalRead(14)) {
		blink(1, 0x000088);
		led.setPixelColor(0, 0x010000);
		led.show();
	}
	delay(200);
	pinMode(SHDN_PIN, INPUT);
	while (true) { // As long as any button is pressed report battery voltage every 60s
		delay(60000);
		client.publish("briefkasten/v", String(analogRead(0) * 0.004324894515).c_str(), (bool)true);
	}
  // Deepsleep doesn't work as the ESP is drawing too less current then to shutdown the system
}

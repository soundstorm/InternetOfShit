#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

const uint8_t SHDN_PIN = 16;
const uint8_t LED_PIN  = 2;
const uint8_t BTN1_PIN = 4;
const uint8_t BTN2_PIN = 14;
const uint8_t BTN3_PIN = 13;
const uint8_t BTN3_PIN_ALT = 12;

boolean button[3];
Adafruit_NeoPixel led = Adafruit_NeoPixel(1, LED_PIN, NEO_RGB + NEO_KHZ800);
bool otaRunning = false;

void setup() {
	digitalWrite(SHDN_PIN, HIGH);
	pinMode(SHDN_PIN, OUTPUT);
	pinMode(BTN1_PIN, INPUT_PULLUP);
	pinMode(BTN2_PIN, INPUT_PULLUP);
	pinMode(BTN3_PIN, INPUT_PULLUP);
	pinMode(BTN3_PIN_ALT, INPUT_PULLUP);
	Serial.begin(115200);
	button[0] = digitalRead(BTN1_PIN);
	button[1] = digitalRead(BTN2_PIN);
	button[2] = digitalRead(BTN3_PIN) & digitalRead(BTN3_PIN_ALT);
	Serial.println(button[0] | (button[1] << 1) | (button[2] << 2));
	led.begin();
	
	WiFi.begin("WIFI", "Password");


	
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
	
	if (button[0] && !(button[1] | button[2])) {
		if (digitalRead(BTN1_PIN) && digitalRead(BTN2_PIN) && digitalRead(BTN3_PIN) && digitalRead(BTN3_PIN_ALT)) {
			Serial.println("OTA");
			delay(1000);
			Serial.println("PORT");
			ArduinoOTA.setPort(8285);
			Serial.println("PASS");
			ArduinoOTA.setPassword((const char *)"Internet0fSh1t");
			Serial.println("HOST");
			ArduinoOTA.setHostname("IoSButton");
			Serial.println("ONSTART");
			ArduinoOTA.onStart([](){
				otaRunning = true;
				if (ArduinoOTA.getCommand() == U_FLASH) {
					led.setPixelColor(0, 128, 128, 0);
				} else {
					led.setPixelColor(0, 0, 0, 128);
				}
				led.show();
			});
			Serial.println("ONERROR");
			ArduinoOTA.onError([](ota_error_t error) {
				if (error == OTA_AUTH_ERROR) {
					blink(1, 0x880000);
				} else if (error == OTA_BEGIN_ERROR) {
					blink(2, 0x880000);
				} else if (error == OTA_CONNECT_ERROR) {
					blink(3, 0x880000);
				} else if (error == OTA_RECEIVE_ERROR) {
					blink(4, 0x880000);
				} else if (error == OTA_END_ERROR) {
					blink(5, 0x880000);
				}
			});
			Serial.println("BEGIN");
			ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
				uint8_t f = progress*128/total;
				led.setPixelColor(0, 128 - f, f, 0);
				led.show();
			});
			Serial.println("BEGIN");
			ArduinoOTA.begin();
			uint32_t st = millis();
			while (millis() < st+60000) {
				if (!otaRunning) {
					f = millis()/10;
					led.setPixelColor(0, abs(f), abs(f), abs(f));
					led.show();
					delay(1);
				}
				ArduinoOTA.handle();
			}
			delay(10000);
			digitalWrite(SHDN_PIN, LOW);
			delay(50);
		}
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
	switch (button[0] | (button[1] << 1) | (button[2] << 2)) {
		// Hier Aktionen einfügen
		case 0b001:
			break;
		case 0b010:
			break;
		case 0b011:
			break;
		case 0b100:
			break;
		case 0b101:
			break;
		case 0b110:
			break;
		case 0b111:
			break;
	}
	for (uint8_t i = 0; i < 3; i++) {
		Serial.print("Button ");
		Serial.print(i+1);
		Serial.print(": ");
		Serial.println(button[i]);
	}
	delay(1000);
	digitalWrite(SHDN_PIN, LOW);
	delay(50);
	ESP.restart();
}

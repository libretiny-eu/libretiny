#include <Arduino.h>

void setup() {
	Serial.begin(115200);
	pinMode(LED_R, OUTPUT);
}

void loop() {
	static unsigned long last = 0;
	if (millis() - last >= 1000) {
		last = millis();
		Serial.print("millis=");
		Serial.println(last);
		digitalWrite(LED_R, !digitalRead(LED_R));
	}
	while (Serial.available()) {
		int c = Serial.read();
		Serial.print("echo: ");
		Serial.write((uint8_t)c);
		Serial.println();
	}
}

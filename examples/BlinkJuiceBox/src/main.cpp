/* Phase 1 RGB cycle for the Enel X JuiceBox 40 (WGM160P host).
 *
 * Walks the three bench-verified RGB channels — LED_R=PB5, LED_G=PB6,
 * LED_B=PD8, all active-high — one at a time, ~700 ms each, and prints the
 * active color on the console UART (PE6/PE7, USART0 LOC1, 115200 8N1).
 */

#include <Arduino.h>

static const uint8_t ch[3] = {LED_R, LED_G, LED_B};

void setup() {
	Serial.begin(115200);
	Serial.println("JuiceBox 40 WGM160P — RGB cycle (R=PB5 G=PB6 B=PD8)");
	for (int i = 0; i < 3; i++)
		pinMode(ch[i], OUTPUT);
}

void loop() {
	const char *name[3] = {"red (PB5)", "green (PB6)", "blue (PD8)"};
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			digitalWrite(ch[j], j == i ? HIGH : LOW);
		Serial.print("active: ");
		Serial.println(name[i]);
		delay(700);
	}
}

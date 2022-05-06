#include <Arduino.h>

extern void pinRemoveMode(pin_size_t pinNumber);
extern void _tone(uint32_t ulPin, unsigned int frequency, unsigned long duration);

void tone(uint32_t ulPin, unsigned int frequency, unsigned long duration) {
	_tone(ulPin, frequency, duration);
}

void noTone(uint32_t ulPin) {
	pinRemoveMode(ulPin);
}

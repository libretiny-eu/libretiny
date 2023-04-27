/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <Arduino.h>

void serialEvent() __attribute__((weak));
bool Serial_available() __attribute__((weak));

void serialEventRun(void) {
	if (Serial_available && serialEvent && Serial_available())
		serialEvent();
}

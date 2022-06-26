/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <Arduino.h>
#include <rtos_pub.h>
#include <sys_rtos.h>

void delayMilliseconds(unsigned long ms) {
	rtos_delay_milliseconds(ms);
}

void delayMicroseconds(unsigned int us) {
	// TODO implement this properly
	us /= 10;
	volatile uint32_t i, j;
	for (i = 0; i < us; i++) {
		for (j = 0; j < 100; j++) {}
	}
}

unsigned long millis() {
	return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

unsigned long micros() {
	// TODO implement this properly
	return millis() * 1000;
}

void yield() {
	vTaskDelay(1);
	taskYIELD();
}

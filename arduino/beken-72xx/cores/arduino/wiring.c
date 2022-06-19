/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <Arduino.h>
#include <rtos_pub.h>
#include <sys_rtos.h>

// TODO work out this conflicting with func/misc/target_util.c
/* void delay(unsigned long ms) {
	rtos_delay_milliseconds(ms);
} */

void delayMicroseconds(unsigned int us) {}

uint32_t millis() {
	return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

void yield() {
	vTaskDelay(1);
	taskYIELD();
}

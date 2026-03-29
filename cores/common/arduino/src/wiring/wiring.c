/* Copyright (c) Kuba Szczodrzyński 2023-05-24. */

#include <Arduino.h>

#if LT_HAS_FREERTOS

__attribute__((weak)) void delay(uint32_t ms) {
	vTaskDelay(pdMS_TO_TICKS(ms));
}

__attribute__((weak)) void yield() {
	runPeriodicTasks();
	vTaskDelay(1);
	taskYIELD();
	lt_wdt_feed();
}

#else

__attribute__((weak)) void yield() {}

#endif

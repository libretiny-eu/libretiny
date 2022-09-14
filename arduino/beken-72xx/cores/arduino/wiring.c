/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <Arduino.h>
#include <arm_arch.h>
#include <bk_timer.h>
#include <bk_timer_pub.h>
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
	// copied from bk_timer_ctrl(), for speeds
	uint32_t timeout = 0;

#if (CFG_SOC_NAME == SOC_BK7231)
#error "Not implemented"
#endif

#if LT_MICROS_HIGH_RES
	REG_WRITE(TIMER0_2_READ_CTL, (BKTIMER0 << 2) | 1);
	while (REG_READ(TIMER0_2_READ_CTL) & 1) {
		timeout++;
		if (timeout > (120 * 1000))
			return 0;
	}
	return millis() * 1000 + REG_READ(TIMER0_2_READ_VALUE) / (CFG_XTAL_FREQUENCE / 1000 / 1000);
#else
#if 0
	REG_WRITE(TIMER3_5_READ_CTL, (BKTIMER3 << 2) | 1);
	while (REG_READ(TIMER3_5_READ_CTL) & 1) {
		timeout++;
		if (timeout > (120 * 1000))
			return 0;
	}
	return millis() * 1000 + REG_READ(TIMER3_5_READ_VALUE) / 32;
#endif
	return millis() * 1000;
#endif
}

void yield() {
	runPeriodicTasks();
	vTaskDelay(1);
	taskYIELD();
}

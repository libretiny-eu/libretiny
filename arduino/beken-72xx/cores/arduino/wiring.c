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


uint32_t get_ticks_count(){
	uint32_t timeout = 0;
	REG_WRITE(TIMER0_2_READ_CTL, (BKTIMER0 << 2) | 1);
	while (REG_READ(TIMER0_2_READ_CTL) & 1) {
		timeout++;
		if (timeout > (120 * 1000))
			return 0;
	}
	return REG_READ(TIMER0_2_READ_VALUE);
}
#define TICKS_PER_US (CFG_XTAL_FREQUENCE / 1000 / 1000)
#define TIMER0_US_OVERFLOW (portTICK_PERIOD_MS * 1000)
#define TICKS_PER_OVERFLOW (TICKS_PER_US * TIMER0_US_OVERFLOW)

void delayMicroseconds(unsigned int us) {
#if LT_MICROS_HIGH_RES
	if (us == 0) return;
	us--;
	uint32_t start_tick = get_ticks_count();
	/* start_tick_2 accounts for the case where the timer counter overflows */
	uint32_t start_tick_2 = start_tick - TICKS_PER_OVERFLOW; 
	uint32_t delay_ticks = TICKS_PER_US * us;
	while (delay_ticks > TICKS_PER_OVERFLOW) {
		// The delay is longer than what the timer can count.
		// Let it overflow until only a fraction of TICKS_PER_OVERFLOW remain.
		while (get_ticks_count() > start_tick) {}
		while (get_ticks_count() < start_tick) {}
		delay_ticks -= TICKS_PER_OVERFLOW;
	}
	while (
		(get_ticks_count() - start_tick < delay_ticks) || // normal case
		(get_ticks_count() - start_tick_2 < delay_ticks) // counter overflow case
	) {}
#else 
	volatile uint32_t i, j;
	for (i = 0; i < us; i++) {
		for (j = 0; j < 6; j++) {}
	}
#endif
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
	return millis() * 1000 + get_ticks_count() / (CFG_XTAL_FREQUENCE / 1000 / 1000);
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

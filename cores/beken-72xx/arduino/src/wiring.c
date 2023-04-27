/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <Arduino.h>
#include <include.h>

#include <arm_arch.h>
#include <bk_timer.h>
#include <bk_timer_pub.h>
#include <rtos_pub.h>
#include <sys_rtos.h>

#define TICKS_PER_US	   (CFG_XTAL_FREQUENCE / 1000 / 1000)
#define US_PER_OVERFLOW	   (portTICK_PERIOD_MS * 1000)
#define TICKS_PER_OVERFLOW (TICKS_PER_US * US_PER_OVERFLOW)

void delayMilliseconds(unsigned long ms) {
	rtos_delay_milliseconds(ms);
}

static uint32_t getTicksCount() {
	// copied from bk_timer_ctrl(), for speeds
	uint32_t timeout = 0;
	REG_WRITE(TIMER0_2_READ_CTL, (BKTIMER0 << 2) | 1);
	while (REG_READ(TIMER0_2_READ_CTL) & 1) {
		timeout++;
		if (timeout > (120 * 1000))
			return 0;
	}
	return REG_READ(TIMER0_2_READ_VALUE);
}

void delayMicroseconds(unsigned int us) {
#if LT_MICROS_HIGH_RES
	if (us == 0)
		return;
	us--;
	uint32_t startTick = getTicksCount();
	/* startTick2 accounts for the case where the timer counter overflows */
	uint32_t startTick2 = startTick - TICKS_PER_OVERFLOW;
	uint32_t delayTicks = TICKS_PER_US * us;
	while (delayTicks > TICKS_PER_OVERFLOW) {
		// The delay is longer than what the timer can count.
		// Let it overflow until only a fraction of TICKS_PER_OVERFLOW remain.
		while (getTicksCount() > startTick) {}
		while (getTicksCount() < startTick) {}
		delayTicks -= TICKS_PER_OVERFLOW;
	}
	while ((getTicksCount() - startTick < delayTicks) || // normal case
		   (getTicksCount() - startTick2 < delayTicks)	 // counter overflow case
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
#if (CFG_SOC_NAME == SOC_BK7231)
#error "Not implemented"
#endif

#if LT_MICROS_HIGH_RES
	static uint32_t lastMillis		= 0;
	static uint32_t correctedMillis = 0;
	static uint32_t lastTicks		= 0;
	uint32_t nowMillis				= millis();
	uint32_t nowTicks				= getTicksCount();
	bool tickOverflow				= nowTicks < lastTicks;
	bool millisUpdated				= nowMillis != lastMillis;
	if (millisUpdated) {
		/* reset artificially corrected millis */
		correctedMillis = nowMillis;
	} else if (tickOverflow) {
		/*
		This can happen if micros is called from within a interruptLock block (interrupts disabled).
		In this case, if the tick counter rolls over, millis() won't be updated, and micros will
		lag by US_PER_OVERFLOW milliseconds (one rollover).
		The workaround only works as long as micros() calls happen within 2ms of eachother.
		WARNING: if interrupts are disabled for more than 2ms, micros() and millis() will temporarily get out of sync.
		*/
		correctedMillis += portTICK_PERIOD_MS;
	}
	lastMillis = nowMillis;
	lastTicks  = nowTicks;
	return correctedMillis * 1000 + nowTicks / (CFG_XTAL_FREQUENCE / 1000 / 1000);
#else
#if 0
	uint32_t timeout = 0;
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

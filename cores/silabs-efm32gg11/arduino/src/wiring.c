/* wiring.c for silabs-efm32gg11.
 *
 * Provides millis()/micros()/delay()/delayMicroseconds(). delay() OVERRIDES the
 * common weak definition (cores/common/arduino/src/wiring/wiring.c) so the
 * call goes through vTaskDelay (cooperative).
 *
 * micros() lazily initialises TIMER0 (HFPERCLK / 64) on first call. TIMER0's
 * clock is already enabled in lt_init_family(); we just init the counter.
 */

#include "ArduinoFamily.h"
#include "FreeRTOS.h"
#include "em_cmu.h"
#include "em_timer.h"
#include "task.h"
#include <Arduino.h>

uint32_t millis(void) {
	return (uint32_t)((xTaskGetTickCount() * 1000U) / configTICK_RATE_HZ);
}

uint32_t micros(void) {
	static bool initialised = false;
	if (!initialised) {
		TIMER_Init_TypeDef init = TIMER_INIT_DEFAULT;
		init.prescale			= timerPrescale64;
		TIMER_Init(TIMER0, &init);
		initialised = true;
	}
	uint64_t ticks = TIMER_CounterGet(TIMER0);
	uint32_t hz	   = CMU_ClockFreqGet(cmuClock_TIMER0) / 64U;
	return (uint32_t)((ticks * 1000000ULL) / hz);
}

void delay(uint32_t ms) {
	vTaskDelay(pdMS_TO_TICKS(ms));
}

void delayMicroseconds(unsigned int us) {
	/* Coarse busy-wait. OK to be preempted; the test is `(now - start) < us`. */
	uint32_t start = micros();
	while ((micros() - start) < us) {
		/* spin */
	}
}

/* Host unit tests for the PWM timer-config math.
 * Pure C, no hardware. Run: make -C cores/silabs-efm32gg11/test/pwm */
#include "lt_pwm.h"
#include <assert.h>

int main(void) {
	// 72 MHz, 50 Hz (20000 us), 16-bit timer (TIMER2): top@/1 = 1.44e6 > 65535,
	// so /32 -> 45000.
	lt_pwm_cfg_t c = lt_pwm_calc(72000000u, 20000u, 8, 0, 16);
	assert(c.prescale_div == 32u);
	assert(c.top == 45000u);

	// Same clock/period on a 32-bit timer (WTIMER): no prescale, full top.
	c = lt_pwm_calc(72000000u, 20000u, 8, 0, 32);
	assert(c.prescale_div == 1u);
	assert(c.top == 1440000u);

	// 8-bit duty scaling against the 32-bit top (1.44e6).
	assert(lt_pwm_calc(72000000u, 20000u, 8, 0, 32).compare == 0u);		   // full off
	assert(lt_pwm_calc(72000000u, 20000u, 8, 255, 32).compare == 1440000u); // full on
	c = lt_pwm_calc(72000000u, 20000u, 8, 128, 32);
	assert(c.compare > 1440000u / 2u - 10000u && c.compare < 1440000u / 2u + 10000u);

	// 10-bit resolution.
	assert(lt_pwm_calc(72000000u, 20000u, 10, 1023, 32).compare == 1440000u);
	c = lt_pwm_calc(72000000u, 20000u, 10, 512, 32);
	assert(c.compare > 1440000u / 2u - 5000u && c.compare < 1440000u / 2u + 5000u);

	// OVERFLOW GUARD: 16-bit resolution, max value, 32-bit top — must equal top
	// with no 32-bit wrap.
	c = lt_pwm_calc(72000000u, 20000u, 16, 65535, 32);
	assert(c.top == 1440000u);
	assert(c.compare == 1440000u);

	// Frequency scaling on the 32-bit timer: 100 Hz halves the top.
	assert(lt_pwm_calc(72000000u, 10000u, 8, 0, 32).top == 720000u);

	// DOESN'T-FIT: 0.5 Hz (2,000,000 us) on the 16-bit timer overflows even /1024
	// -> clamp to max prescaler + max top.
	c = lt_pwm_calc(72000000u, 2000000u, 8, 255, 16);
	assert(c.prescale_div == 1024u);
	assert(c.top == 65535u);
	assert(c.compare == 65535u); // full on still maps to top after clamp

	// Value over range clamps to top.
	assert(lt_pwm_calc(72000000u, 20000u, 8, 999, 32).compare == 1440000u);

	return 0;
}

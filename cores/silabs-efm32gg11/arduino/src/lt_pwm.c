/* Copyright (c) WGM160P-LibreTiny port 2026-06-17. */
#include "lt_pwm.h"

lt_pwm_cfg_t lt_pwm_calc(
	uint32_t timer_clk_hz, uint32_t period_us, uint8_t res_bits, uint32_t value, uint8_t timer_bits
) {
	lt_pwm_cfg_t cfg;

	uint64_t top_max = (timer_bits >= 32) ? 0xFFFFFFFFULL : ((1ULL << timer_bits) - 1ULL);
	uint32_t div	 = 1u;
	uint64_t top	 = 0;
	for (;;) {
		top = (uint64_t)timer_clk_hz * period_us / 1000000ULL / div;
		if (top <= top_max || div >= LT_PWM_MAX_PRESCALE)
			break;
		div <<= 1;
	}
	if (top > top_max)
		top = top_max; // doesn't fit even at /1024: clamp
	if (top == 0)
		top = 1; // degenerate (very high freq): keep a usable period
	cfg.prescale_div = div;
	cfg.top			 = (uint32_t)top;

	uint32_t maxval = (res_bits >= 32) ? 0xFFFFFFFFu : ((1u << res_bits) - 1u);
	if (maxval == 0)
		maxval = 1;
	if (value > maxval)
		value = maxval;
	uint64_t cmp = (uint64_t)value * top / maxval;
	if (cmp > top)
		cmp = top;
	cfg.compare = (uint32_t)cmp;
	return cfg;
}

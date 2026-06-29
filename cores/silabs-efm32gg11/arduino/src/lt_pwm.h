/* Copyright (c) WGM160P-LibreTiny port 2026-06-17. */
/* lt_pwm.h — pure PWM timer-config math for silabs-efm32gg11 analogWrite().
 * No emlib/Arduino deps, so it is host-unit-tested (cores/.../test/pwm). */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LT_PWM_MAX_PRESCALE 1024u // EFM32 TIMER prescaler max (power of two)

typedef struct {
	uint32_t prescale_div; // 1,2,4,…,1024
	uint32_t top;		   // TIMER TOP (period in timer ticks)
	uint32_t compare;	   // CCx compare (duty), in [0, top]
} lt_pwm_cfg_t;

// Compute the timer configuration for one PWM channel. All intermediates are
// 64-bit (the products overflow 32-bit). Picks the smallest power-of-two
// prescaler in {1..1024} so `top` fits `timer_bits` (16 for TIMER2, 32 for
// WTIMER); if it does not fit even at /1024, clamps top to the counter max.
// compare = value/(2^res_bits-1) * top, clamped to [0, top]; value 0 => 0
// (full off), value at full scale => top (full on, active-high).
lt_pwm_cfg_t lt_pwm_calc(
	uint32_t timer_clk_hz, uint32_t period_us, uint8_t res_bits, uint32_t value, uint8_t timer_bits
);

#ifdef __cplusplus
}
#endif

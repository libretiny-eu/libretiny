/* Copyright (c) Kuba Szczodrzyński 2023-05-24. */

#pragma once

#include <Arduino.h>
#include <sdk_private.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum lt_pwm_state_tag { LT_PWM_STOPPED, LT_PWM_RUNNING, LT_PWM_PAUSED } lt_pwm_state_t;

struct PinData_s {
	pwm_param_t pwm;
	lt_pwm_state_t pwmState;
	PinMode gpioMode;
	PinStatus irqMode;
	void *irqHandler;
	void *irqParam;
	bool irqChange;
};

#ifdef __cplusplus
} // extern "C"
#endif

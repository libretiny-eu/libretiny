/* Copyright (c) Kuba Szczodrzyński 2023-05-24. */

#pragma once

#include <ArduinoPrivate.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum lt_pwm_state_tag { LT_PWM_STOPPED, LT_PWM_RUNNING, LT_PWM_PAUSED } lt_pwm_state_t;

struct PinData_s {
#if CFG_BDK_USE_NEW_PWM_DRIVER
	pwm_param_st pwm;
#else
	pwm_param_t pwm;
#endif
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

/* Copyright (c) Kuba Szczodrzyński 2023-05-24. */

#pragma once

#include <Arduino.h>
#include <sdk_private.h>

#ifdef __cplusplus
extern "C" {
#endif

struct PinData_s {
	gpio_t *gpio;
	gpio_irq_t *irq;
	pwmout_t *pwm;
	PinMode gpioMode;
	PinStatus irqMode;
	void *irqHandler;
	void *irqParam;
};

#ifdef __cplusplus
} // extern "C"
#endif

/* Copyright (c) Etienne Le Cousin 2024-02-17. */

#pragma once

#include <Arduino.h>
#include <sdk_private.h>

#ifdef __cplusplus
extern "C" {
#endif

struct PinData_s {
	gpio_init_t_def *gpio;
	PinMode gpioMode;
	uint32_t gpio_base;
	PinStatus irqMode;
	void *irqHandler;
	void *irqParam;
};

#define GPIO_GET_PORT(pin) (pin >> 4)
#define GPIO_GET_PIN(pin)  ((gpio_pin_t)(1 << (pin & 0xF)))
#define GPIO_GET_BASE(pin) (GPIO_GET_PORT(pin) == 1 ? GPIOB_BASE : GPIOA_BASE)

#ifdef __cplusplus
} // extern "C"
#endif

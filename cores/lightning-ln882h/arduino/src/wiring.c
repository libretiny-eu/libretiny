/* Copyright (c) Etienne Le Cousin 2024-02-17. */

#include "wiring_private.h"

#ifndef portNVIC_SYSTICK_CURRENT_VALUE_REG
#define portNVIC_SYSTICK_CURRENT_VALUE_REG (*((volatile uint32_t *)0xe000e018))
#endif

void delayMicroseconds(unsigned int us) {
	int i;
	uint32_t t0, tn;
	int dfactor = 20 * us - 10 + (81 * us / 100);

	if (us > 100) {
		t0 = micros();
		do {
			tn = micros();
		} while (tn >= t0 && tn < (t0 + us - 1));
	} else {
		for (i = 0; i < dfactor; i++) {
			asm("nop");
		}
	}
}

unsigned long millis(void) {
	return (__get_IPSR() == 0) ? xTaskGetTickCount() : xTaskGetTickCountFromISR();
}

unsigned long micros(void) {
	uint32_t tick1, tick2;
	uint32_t us;
	uint32_t tick_per_us = F_CPU / 1000;

	if (__get_IPSR() == 0) {
		tick1 = xTaskGetTickCount();
		us	  = portNVIC_SYSTICK_CURRENT_VALUE_REG;
		tick2 = xTaskGetTickCount();
	} else {
		tick1 = xTaskGetTickCountFromISR();
		us	  = portNVIC_SYSTICK_CURRENT_VALUE_REG;
		tick2 = xTaskGetTickCountFromISR();
	}

	if (tick1 == tick2) {
		return tick1 * 1000 - us * 1000 / tick_per_us;
	} else if ((us * 1000 / tick_per_us) < 500) {
		return tick1 * 1000 - us * 1000 / tick_per_us;
	} else {
		return tick1 * 1000;
	}
}

void pinRemoveMode(PinInfo *pin, uint32_t mask) {
	PinData *data = pinData(pin);
	if ((mask & PIN_GPIO) && (pin->enabled & PIN_GPIO)) {
		hal_gpio_pin_direction_set(data->gpio_base, data->gpio->pin, GPIO_INPUT);
		free(data->gpio);
		pinDisable(pin, PIN_GPIO);
	}
	if ((mask & PIN_IRQ) && (pin->enabled & PIN_IRQ)) {
		data->irqHandler = NULL;
		hal_gpio_pin_it_en(data->gpio_base, data->gpio->pin, HAL_DISABLE);
		pinDisable(pin, PIN_IRQ);
	}
}

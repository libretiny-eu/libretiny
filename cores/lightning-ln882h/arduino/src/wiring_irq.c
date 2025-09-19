/* Copyright (c) Etienne Le Cousin 2024-03-04. */

#include "wiring_private.h"

void GPIOA_IRQHandler() {
	for (pin_size_t pinNumber = 0; pinNumber < 16; pinNumber++) {
		gpio_pin_t gpio = GPIO_GET_PIN(pinNumber);

		if (hal_gpio_pin_get_it_flag(GPIOA_BASE, gpio) == HAL_SET) {
			hal_gpio_pin_clr_it_flag(GPIOA_BASE, gpio);

			PinInfo *pin = pinInfo(pinNumber);
			if (!pin)
				continue;

			PinData *data = pinData(pin);
			if (!data->irqHandler)
				continue;
			if (!data->irqParam)
				((voidFuncPtr)data->irqHandler)();
			else
				((voidFuncPtrParam)data->irqHandler)(data->irqParam);
		}
	}
}

void GPIOB_IRQHandler() {
	for (pin_size_t pinNumber = 16; pinNumber < 32; pinNumber++) {
		gpio_pin_t gpio = GPIO_GET_PIN(pinNumber);

		if (hal_gpio_pin_get_it_flag(GPIOB_BASE, gpio) == HAL_SET) {
			hal_gpio_pin_clr_it_flag(GPIOB_BASE, gpio);

			PinInfo *pin = pinInfo(pinNumber);
			if (!pin)
				continue;

			PinData *data = pinData(pin);
			if (!data->irqHandler)
				continue;
			if (!data->irqParam)
				((voidFuncPtr)data->irqHandler)();
			else
				((voidFuncPtrParam)data->irqHandler)(data->irqParam);
		}
	}
}

void attachInterruptParam(pin_size_t interruptNumber, voidFuncPtrParam callback, PinStatus mode, void *param) {
	pinCheckGetData(interruptNumber, PIN_IRQ, );

	data->irqHandler = callback;
	data->irqParam	 = param;

	if (pinEnabled(pin, PIN_IRQ) && data->irqMode == mode)
		return;

	// GPIO can't be used together with PWM
	pinRemoveMode(pin, PIN_PWM);

	uint32_t event = 0;
	switch (mode) {
		case LOW:
			LT_W("LOW interrupts not supported, setting FALLING instead");
		case FALLING:
			event = GPIO_INT_FALLING;
			break;
		case HIGH:
			LT_W("HIGH interrupts not supported, setting RISING instead");
		case RISING:
			event = GPIO_INT_RISING;
			break;
		case CHANGE:
			event = GPIO_INT_RISING_FALLING;
			break;
		default:
			return;
	}
	pinEnable(pin, PIN_IRQ);
	data->irqMode = mode;

	hal_gpio_pin_it_cfg(data->gpio_base, data->gpio->pin, event);
	hal_gpio_pin_it_en(data->gpio_base, data->gpio->pin, HAL_ENABLE);
	uint16_t IRQForPin = GPIO_GET_PORT(pin->gpio) == PORT_A ? GPIOA_IRQn : GPIOB_IRQn;
	NVIC_SetPriority(IRQForPin, 1);
	NVIC_EnableIRQ(IRQForPin);
}

void detachInterrupt(pin_size_t interruptNumber) {
	pinCheckGetData(interruptNumber, PIN_IRQ, );

	hal_gpio_pin_it_en(data->gpio_base, data->gpio->pin, HAL_DISABLE);
	pinModeRemove(interruptNumber, PIN_IRQ);
}

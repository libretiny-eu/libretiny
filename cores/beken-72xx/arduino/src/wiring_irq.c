/* Copyright (c) Kuba Szczodrzyński 2022-07-31. */

#include "wiring_private.h"

static void irqHandler(unsigned char gpio) {
	PinInfo *pin = pinByGpio(gpio);
	if (pin == NULL)
		return;
	PinData *data = pinData(pin);
	if (!data->irqHandler)
		return;
	if (data->irqChange) {
		if (data->gpioMode == INPUT_PULLDOWN) {
			data->gpioMode = INPUT_PULLUP;
			gpio_int_enable(pin->gpio, GPIO_INT_LEVEL_FALLING, irqHandler);
		} else if (data->gpioMode == INPUT_PULLUP) {
			data->gpioMode = INPUT_PULLDOWN;
			gpio_int_enable(pin->gpio, GPIO_INT_LEVEL_RISING, irqHandler);
		}
	}
	if (!data->irqParam)
		((voidFuncPtr)data->irqHandler)();
	else
		((voidFuncPtrParam)data->irqHandler)(data->irqParam);
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
	bool change	   = false;
	switch (mode) {
		case LOW:
			event = GPIO_INT_LEVEL_LOW;
			break;
		case HIGH:
			event = GPIO_INT_LEVEL_HIGH;
			break;
		case FALLING:
			event = GPIO_INT_LEVEL_FALLING;
			break;
		case RISING:
			event = GPIO_INT_LEVEL_RISING;
			break;
		case CHANGE:
			event  = GPIO_INT_LEVEL_FALLING;
			change = true;
			break;
		default:
			return;
	}
	pinEnable(pin, PIN_IRQ);
	data->irqMode	= mode;
	data->irqChange = change;

	gpio_int_enable(pin->gpio, event, irqHandler);
}

void detachInterrupt(pin_size_t interruptNumber) {
	pinModeRemove(interruptNumber, PIN_IRQ);
}

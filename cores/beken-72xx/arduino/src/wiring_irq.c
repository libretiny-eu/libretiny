/* Copyright (c) Kuba Szczodrzyński 2022-07-31. */

#include <Arduino.h>

#include <gpio_pub.h>

static void *irqHandlerList[PINS_COUNT] = {NULL};
static void *irqHandlerArgs[PINS_COUNT] = {NULL};
static bool irqChangeList[PINS_COUNT];

static void irqHandler(unsigned char gpio) {
	PinInfo *pin = pinByGpio(gpio);
	if (pin == NULL)
		return;
	uint32_t index = pinIndex(pin);
	if (!irqHandlerList[index])
		return;
	if (irqChangeList[index]) {
		if (pin->mode == INPUT_PULLDOWN) {
			pin->mode = INPUT_PULLUP;
			gpio_int_enable(pin->gpio, GPIO_INT_LEVEL_FALLING, irqHandler);
		} else if (pin->mode == INPUT_PULLUP) {
			pin->mode = INPUT_PULLDOWN;
			gpio_int_enable(pin->gpio, GPIO_INT_LEVEL_RISING, irqHandler);
		}
	}
	if (irqHandlerArgs[index] == NULL) {
		((voidFuncPtr)irqHandlerList[index])();
	} else {
		((voidFuncPtrParam)irqHandlerList[index])(irqHandlerArgs[index]);
	}
}

void attachInterrupt(pin_size_t interruptNumber, voidFuncPtr callback, PinStatus mode) {
	attachInterruptParam(interruptNumber, (voidFuncPtrParam)callback, mode, NULL);
}

void attachInterruptParam(pin_size_t interruptNumber, voidFuncPtrParam callback, PinStatus mode, void *param) {
	PinInfo *pin = pinInfo(interruptNumber);
	if (!pin)
		return;
	if (!pinSupported(pin, PIN_IRQ))
		return;
	uint32_t index	= pinIndex(pin);
	uint32_t event	= 0;
	PinMode modeNew = 0;
	bool change		= 0;

	switch (mode) {
		case LOW:
			event	= GPIO_INT_LEVEL_LOW;
			modeNew = INPUT_PULLUP;
			change	= false;
			break;
		case HIGH:
			event	= GPIO_INT_LEVEL_HIGH;
			modeNew = INPUT_PULLDOWN;
			change	= false;
			break;
		case FALLING:
			event	= GPIO_INT_LEVEL_FALLING;
			modeNew = INPUT_PULLUP;
			change	= false;
			break;
		case RISING:
			event	= GPIO_INT_LEVEL_RISING;
			modeNew = INPUT_PULLDOWN;
			change	= false;
			break;
		case CHANGE:
			event	= GPIO_INT_LEVEL_FALLING;
			modeNew = INPUT_PULLUP;
			change	= true;
			break;
		default:
			return;
	}
	irqHandlerList[index] = callback;
	irqHandlerArgs[index] = param;
	irqChangeList[index]  = change;
	gpio_int_enable(pin->gpio, event, irqHandler);
	pin->enabled |= PIN_IRQ | PIN_GPIO;
	pin->mode = modeNew;
}

void detachInterrupt(pin_size_t interruptNumber) {
	PinInfo *pin = pinInfo(interruptNumber);
	if (!pin)
		return;
	if (!pinSupported(pin, PIN_IRQ))
		return;
	uint32_t index		  = pinIndex(pin);
	irqHandlerList[index] = NULL;
	irqHandlerArgs[index] = NULL;
	irqChangeList[index]  = false;
	gpio_int_disable(pin->gpio);
	pin->enabled &= ~PIN_IRQ;
}

/* Copyright (c) Kuba Szczodrzyński 2022-04-23. */

#include "wiring_private.h"

static void gpioIrqHandler(uint32_t id, gpio_irq_event event) {
	// id is pin data
	PinData *data = (PinData *)id;
	if (!data->irqHandler)
		return;
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

#if LT_RTL8720C
	// apparently IRQ can't be used with any kind of pull-up/down
	// TODO verify if it can be used on AmebaZ
	pinRemoveMode(pin, PIN_PWM | PIN_GPIO);
#else
	// GPIO can't be used together with PWM
	pinRemoveMode(pin, PIN_PWM);
#endif

	gpio_irq_t *irq = data->irq;
	if (!irq) {
		// allocate memory if pin not used before
		data->irq = irq = malloc(sizeof(gpio_irq_t));
		if (gpio_irq_init(irq, pin->gpio, gpioIrqHandler, (uint32_t)data) != 0) {
			LT_W("IRQ init failed");
			free(data->irq);
			data->irq = NULL;
			return;
		}
		pinEnable(pin, PIN_IRQ);
	}

	gpio_irq_event event;
	switch (mode) {
		case LOW:
			event = IRQ_LOW;
			break;
		case HIGH:
			event = IRQ_HIGH;
			break;
		case FALLING:
			event = IRQ_FALL;
			break;
		case RISING:
			event = IRQ_RISE;
			break;
		case CHANGE:
#if LT_RTL8720C
			event = IRQ_FALL_RISE;
// Prevents Change interrupt errors on RTL8710B chips.
#elif LT_RTL8710B
			event = IRQ_RISE;
#else
			LT_W("CHANGE interrupts not supported");
#endif
			break;
		default:
			return;
	}
	data->irqMode = mode;

	gpio_irq_set(irq, event, 1);
	gpio_irq_enable(irq);
}

void detachInterrupt(pin_size_t interruptNumber) {
	pinModeRemove(interruptNumber, PIN_IRQ);
}

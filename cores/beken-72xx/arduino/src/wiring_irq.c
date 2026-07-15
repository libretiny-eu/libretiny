/* Copyright (c) Kuba Szczodrzyński 2022-07-31. */

#include <ArduinoPrivate.h>

#if LT_BK7238
// On BK7238 the GPIO edge interrupt reaches the ISR after the pin has already
// settled back to its idle level for a fast pulse train (IRQ latency can exceed
// the pulse high-time). digitalRead() inside the ISR therefore returns a stale
// level, so consumers that infer the edge direction by reading the pin (e.g.
// ESPHome pulse_counter / hlw8012) misclassify every edge and count nothing.
// This is not observed on BK7231x, so the workaround is scoped to BK7238 only.
//
// Fix: the ISR already knows which edge fired (data->irqMode, before it flips for
// CHANGE emulation), which implies the pin's logical level. Cache it here and
// return it from digitalRead() while executing that pin's interrupt handler.
volatile int lt_isr_level		   = -1;	  // logical level during ISR (-1 = none)
volatile unsigned char lt_isr_gpio = 0xff;	  // gpio whose ISR is running
#endif

static void irqHandler(unsigned char gpio) {
	PinInfo *pin = pinByGpio(gpio);
	if (pin == NULL)
		return;
	PinData *data = pinData(pin);
	if (!data->irqHandler)
		return;
#if LT_BK7238
	// The edge that just fired == the currently-armed edge, which implies the
	// pin's logical level (unreliable to read directly in-ISR on this SoC).
	if (data->irqMode == RISING)
		lt_isr_level = HIGH;
	else if (data->irqMode == FALLING)
		lt_isr_level = LOW;
	else
		lt_isr_level = -1;
	lt_isr_gpio = gpio;
#endif
	if (!data->irqParam)
		((voidFuncPtr)data->irqHandler)();
	else
		((voidFuncPtrParam)data->irqHandler)(data->irqParam);
#if LT_BK7238
	lt_isr_gpio	 = 0xff;
	lt_isr_level = -1;
#endif
	if (data->irqChange) {
		if (data->irqMode == RISING) {
			data->irqMode = FALLING;
			gpio_int_enable(pin->gpio, GPIO_INT_LEVEL_FALLING, irqHandler);
		} else {
			data->irqMode = RISING;
			gpio_int_enable(pin->gpio, GPIO_INT_LEVEL_RISING, irqHandler);
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
			if (gpio_input(pin->gpio)) {
				event = GPIO_INT_LEVEL_FALLING;
				mode  = FALLING;
			} else {
				event = GPIO_INT_LEVEL_RISING;
				mode  = RISING;
			}
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

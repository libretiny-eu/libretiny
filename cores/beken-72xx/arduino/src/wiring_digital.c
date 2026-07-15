/* Copyright (c) Kuba Szczodrzyński 2022-06-20. */

#include <ArduinoPrivate.h>

void pinMode(pin_size_t pinNumber, PinMode pinMode) {
	pinCheckGetData(pinNumber, PIN_GPIO, );

	if (pinEnabled(pin, PIN_GPIO) && data->gpioMode == pinMode)
		return;

	// GPIO can't be used together with PWM
	pinRemoveMode(pin, PIN_PWM);

	switch (pinMode) {
		case INPUT:
			gpio_config(pin->gpio, GMODE_INPUT);
			break;
		case OUTPUT:
			gpio_config(pin->gpio, GMODE_OUTPUT);
			break;
		case INPUT_PULLUP:
			gpio_config(pin->gpio, GMODE_INPUT_PULLUP);
			break;
		case INPUT_PULLDOWN:
			gpio_config(pin->gpio, GMODE_INPUT_PULLDOWN);
			break;
		case OUTPUT_OPENDRAIN:
			gpio_config(pin->gpio, GMODE_SET_HIGH_IMPENDANCE);
			break;
		default:
			return;
	}
	pinEnable(pin, PIN_GPIO);
	data->gpioMode = pinMode;
}

void digitalWrite(pin_size_t pinNumber, PinStatus status) {
	pinCheckGetData(pinNumber, PIN_GPIO, );
	pinSetOutputPull(pin, data, pinNumber, status);
	gpio_output(pin->gpio, !!status);
}

#if LT_BK7238
// Set by the GPIO ISR (wiring_irq.c) so digitalRead() can return the true
// just-transitioned level of the pin being serviced, which is otherwise
// unreliable to read from the GPIO registers inside a fast-pulse ISR on BK7238.
extern volatile int lt_isr_level;
extern volatile unsigned char lt_isr_gpio;
#endif

PinStatus digitalRead(pin_size_t pinNumber) {
	pinCheckGetData(pinNumber, PIN_GPIO, LOW);
#if LT_BK7238
	// If we're inside this pin's interrupt handler, return the edge-implied level.
	if (lt_isr_level >= 0 && pin->gpio == lt_isr_gpio)
		return (PinStatus)lt_isr_level;
#endif
	pinSetInputMode(pin, data, pinNumber);
	return gpio_input(pin->gpio);
}

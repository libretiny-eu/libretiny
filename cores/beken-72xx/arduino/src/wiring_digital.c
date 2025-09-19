/* Copyright (c) Kuba Szczodrzyński 2022-06-20. */

#include "wiring_private.h"

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

PinStatus digitalRead(pin_size_t pinNumber) {
	pinCheckGetData(pinNumber, PIN_GPIO, LOW);
	pinSetInputMode(pin, data, pinNumber);
	return gpio_input(pin->gpio);
}

/* Copyright (c) Kuba Szczodrzyński 2022-06-20. */

#include <Arduino.h>

#include <gpio_pub.h>

void pinMode(pin_size_t pinNumber, PinMode pinMode) {
	PinInfo *pin = pinInfo(pinNumber);
	if (!pin)
		return;
	if (!pinSupported(pin, PIN_GPIO))
		return;
	if (pinEnabled(pin, PIN_PWM))
		// disable PWM before using the pin
		analogWrite(pinNumber, 0);
	if (pinEnabled(pin, PIN_GPIO) && pin->mode == pinMode)
		return;
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
	}
	pin->enabled |= PIN_GPIO;
	pin->mode = pinMode;
}

void digitalWrite(pin_size_t pinNumber, PinStatus status) {
	// verify level is 0 or 1
	if (status > HIGH)
		return;
	PinInfo *pin = pinInfo(pinNumber);
	if (!pin)
		return;
	// pin is not GPIO yet or not OUTPUT; enable or disable input pullup
	if (!pinEnabled(pin, PIN_GPIO) || !pinIsOutput(pin)) {
		pinMode(pinNumber, status * INPUT_PULLUP);
		return;
	}
	// write the new state
	gpio_output(pin->gpio, status);
}

PinStatus digitalRead(pin_size_t pinNumber) {
	PinInfo *pin = pinInfo(pinNumber);
	if (!pin)
		return 0;
	// pin is not GPIO yet or not INPUT; change the mode
	if (!pinEnabled(pin, PIN_GPIO) || !pinIsInput(pin))
		pinMode(pinNumber, INPUT);
	// read the value
	return gpio_input(pin->gpio);
}

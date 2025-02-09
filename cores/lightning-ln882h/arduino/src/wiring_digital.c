/* Copyright (c) Etienne Le Cousin 2024-02-17. */

#include "wiring_private.h"

void pinMode(pin_size_t pinNumber, PinMode pinMode) {
	pinCheckGetData(pinNumber, PIN_GPIO, );

	if (pinEnabled(pin, PIN_GPIO) && data->gpioMode == pinMode)
		return;

	// GPIO can't be used together with PWM
	pinRemoveMode(pin, PIN_PWM);

	gpio_init_t_def *gpio = data->gpio;
	if (!gpio) {
		// allocate memory if pin not used before
		data->gpio = gpio = malloc(sizeof(gpio_init_t_def));
		pinEnable(pin, PIN_GPIO);
	}

	gpio->pin		= GPIO_GET_PIN(pinNumber);
	gpio->speed		= GPIO_NORMAL_SPEED;
	data->gpio_base = GPIO_GET_BASE(pinNumber);
	data->gpioMode	= pinMode;

	switch (pinMode) {
		case INPUT:
			gpio->dir  = GPIO_INPUT;
			gpio->pull = GPIO_PULL_NONE;
			break;
		case INPUT_PULLDOWN:
			gpio->dir  = GPIO_INPUT;
			gpio->pull = GPIO_PULL_DOWN;
			break;
		case INPUT_PULLUP:
			gpio->dir  = GPIO_INPUT;
			gpio->pull = GPIO_PULL_UP;
			break;
		case OUTPUT:
			gpio->dir  = GPIO_OUTPUT;
			gpio->pull = GPIO_PULL_NONE;
			break;
		case OUTPUT_OPENDRAIN:
			gpio->dir  = GPIO_INPUT;
			gpio->pull = GPIO_PULL_NONE;
			break;
		default:
			return;
	}

	hal_gpio_init(data->gpio_base, gpio);
}

void digitalWrite(pin_size_t pinNumber, PinStatus status) {
	pinCheckGetData(pinNumber, PIN_GPIO, );
	pinSetOutputPull(pin, data, pinNumber, status);
	if (status)
		hal_gpio_pin_set(data->gpio_base, data->gpio->pin);
	else
		hal_gpio_pin_reset(data->gpio_base, data->gpio->pin);
}

PinStatus digitalRead(pin_size_t pinNumber) {
	pinCheckGetData(pinNumber, PIN_GPIO, LOW);
	pinSetInputMode(pin, data, pinNumber);
	return hal_gpio_pin_input_read(data->gpio_base, data->gpio->pin);
}

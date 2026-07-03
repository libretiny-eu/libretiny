/* Copyright (c) Kuba Szczodrzyński 2022-04-23. */

#include <ArduinoPrivate.h>

void pinMode(pin_size_t pinNumber, PinMode pinMode) {
	pinCheckGetData(pinNumber, PIN_GPIO, );

	if (pinEnabled(pin, PIN_GPIO) && data->gpioMode == pinMode)
		return;

#if LT_RTL8720C
	// apparently IRQ can't be used with any kind of pull-up/down
	// TODO verify if it can be used on AmebaZ
	pinRemoveMode(pin, PIN_PWM | PIN_IRQ);
#else
	// GPIO can't be used together with PWM
	pinRemoveMode(pin, PIN_PWM);
#endif

	gpio_t *gpio = data->gpio;
	if (!gpio) {
		// allocate memory if pin not used before
		data->gpio = gpio = malloc(sizeof(gpio_t));
		gpio_init(gpio, pin->gpio);
		pinEnable(pin, PIN_GPIO);
	}

	PinDirection dir;
	PinModeRTL mode;

	switch (pinMode) {
		case INPUT:
			dir	 = PIN_INPUT;
			mode = PullNone;
			break;
		case INPUT_PULLDOWN:
			dir	 = PIN_INPUT;
			mode = PullDown;
			break;
		case INPUT_PULLUP:
			dir	 = PIN_INPUT;
			mode = PullUp;
			break;
		case OUTPUT:
			dir	 = PIN_OUTPUT;
			mode = PullNone;
			break;
		case OUTPUT_OPENDRAIN:
			dir	 = PIN_OUTPUT;
			mode = OpenDrain;
			break;
		default:
			return;
	}
	data->gpioMode = pinMode;

	gpio_dir(gpio, dir);
	gpio_mode(gpio, mode);
}

void digitalWrite(pin_size_t pinNumber, PinStatus status) {
	pinCheckGetData(pinNumber, PIN_GPIO, );
	pinSetOutputPull(pin, data, pinNumber, status);
	gpio_write(data->gpio, !!status);
}

PinStatus digitalRead(pin_size_t pinNumber) {
	pinCheckGetData(pinNumber, PIN_GPIO, LOW);
#if LT_RTL8720C
	// a pin can't be in GPIO and IRQ mode at the same time (pinmux conflict),
	// so read IRQ-mode pins through the IRQ object instead; falling through
	// to pinMode(INPUT) here would detach the interrupt and free its object -
	// fatal when digitalRead() is called from within the ISR itself
	// (e.g. ESPHome's pulse_counter reads the pin in its IRQ handler)
	if (pinEnabled(pin, PIN_IRQ) && data->irq)
		return gpio_irq_read(data->irq) ? HIGH : LOW;
#endif
	pinSetInputMode(pin, data, pinNumber);
	return gpio_read(data->gpio);
}

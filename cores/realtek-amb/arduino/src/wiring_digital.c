#include <Arduino.h>
#include <sdk_private.h>

extern void *gpio_pin_struct[PINS_COUNT];

void pinRemoveMode(pin_size_t pinNumber) {
	PinInfo *pin = pinInfo(pinNumber);
	if (!pin)
		return;
	if (pinEnabled(pin, PIN_PWM)) {
		pwmout_t *obj = (pwmout_t *)gpio_pin_struct[pinNumber];
		pwmout_free(obj);
	}
	if (pinEnabled(pin, PIN_GPIO)) {
		gpio_t *obj = (gpio_t *)gpio_pin_struct[pinNumber];
		gpio_deinit(obj, pin->gpio);
		free(obj);
	}
	if (pinEnabled(pin, PIN_IRQ)) {
		gpio_irq_t *obj = (gpio_irq_t *)gpio_pin_struct[pinNumber];
		gpio_irq_deinit(obj);
		free(obj);
	}
	gpio_pin_struct[pinNumber] = NULL;
	pin->enabled			   = PIN_NONE;
}

void pinMode(pin_size_t pinNumber, PinMode pinMode) {
	PinInfo *pin = pinInfo(pinNumber);
	if (!pin)
		return;

	if (pinEnabled(pin, PIN_GPIO) && pin->mode == pinMode)
		// Nothing changes in pin mode
		return;

	if (!pinSupported(pin, PIN_GPIO))
		// cannot set ADC as I/O
		return;

	/* if (pin->enabled == PIN_PWM) {
		// If this pin has been configured as PWM, then it cannot change to another mode
		return;
	} */

	if (pin->enabled != PIN_GPIO)
		// pin mode changes; deinit gpio and free memory
		pinRemoveMode(pinNumber);

	gpio_t *gpio;

	if (pin->enabled == PIN_NONE) {
		// allocate memory if pin not used before
		gpio					   = malloc(sizeof(gpio_t));
		gpio_pin_struct[pinNumber] = gpio;
		gpio_init(gpio, pin->gpio);
		pin->enabled = PIN_GPIO;
	} else {
		// pin already used as gpio
		gpio = (gpio_t *)gpio_pin_struct[pinNumber];
	}
	pin->mode = pinMode;

	PinDirection dir;
	PinMode mode;

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

	gpio_dir(gpio, dir);
	gpio_mode(gpio, mode);
}

void digitalWrite(pin_size_t pinNumber, PinStatus status) {
	PinInfo *pin = pinInfo(pinNumber);
	if (!pin)
		return;
	if (pin->enabled != PIN_GPIO)
		return;

	gpio_t *gpio = (gpio_t *)gpio_pin_struct[pinNumber];
	gpio_write(gpio, status);
}

PinStatus digitalRead(pin_size_t pinNumber) {
	PinInfo *pin = pinInfo(pinNumber);
	if (!pin)
		return;
	if (pin->enabled != PIN_GPIO)
		return;

	gpio_t *gpio = (gpio_t *)gpio_pin_struct[pinNumber];
	return gpio_read(gpio);
}

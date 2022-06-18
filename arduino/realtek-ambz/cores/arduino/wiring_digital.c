#include <Arduino.h>
#include <gpio_api.h>
#include <gpio_irq_api.h>
#include <gpio_irq_ex_api.h>
#include <pwmout_api.h>

extern void *gpio_pin_struct[PINS_COUNT];

bool pinInvalid(pin_size_t pinNumber) {
	return pinNumber < 0 || pinNumber >= PINS_COUNT || pinTable[pinNumber].gpio == NC;
}

void pinRemoveMode(pin_size_t pinNumber) {
	if (pinInvalid(pinNumber))
		return;
	if (pinTable[pinNumber].types & PIN_PWM) {
		pwmout_t *obj = (pwmout_t *)gpio_pin_struct[pinNumber];
		pwmout_free(obj);
	}
	if (pinTable[pinNumber].types & PIN_GPIO) {
		gpio_t *obj = (gpio_t *)gpio_pin_struct[pinNumber];
		gpio_deinit(obj, pinTable[pinNumber].gpio);
		free(obj);
	}
	if (pinTable[pinNumber].types & PIN_IRQ) {
		gpio_irq_t *obj = (gpio_irq_t *)gpio_pin_struct[pinNumber];
		gpio_irq_deinit(obj);
		free(obj);
	}
	gpio_pin_struct[pinNumber] = NULL;
	pinTable[pinNumber].types  = PIN_NONE;
}

void pinMode(pin_size_t pinNumber, PinModeArduino pinMode) {
	if (pinInvalid(pinNumber))
		return;

	if (pinTable[pinNumber].types == PIN_GPIO && pinTable[pinNumber].mode == pinMode)
		// Nothing changes in pin mode
		return;

	if ((pinTable[pinNumber].features & PIN_GPIO) != PIN_GPIO)
		// cannot set ADC as I/O
		return;

	/* if (pinTable[pinNumber].types == PIN_PWM) {
		// If this pin has been configured as PWM, then it cannot change to another mode
		return;
	} */

	if (pinTable[pinNumber].types != PIN_GPIO)
		// pin mode changes; deinit gpio and free memory
		pinRemoveMode(pinNumber);

	gpio_t *gpio;

	if (pinTable[pinNumber].types == PIN_NONE) {
		// allocate memory if pin not used before
		gpio					   = malloc(sizeof(gpio_t));
		gpio_pin_struct[pinNumber] = gpio;
		gpio_init(gpio, pinTable[pinNumber].gpio);
		pinTable[pinNumber].types = PIN_GPIO;
	} else {
		// pin already used as gpio
		gpio = (gpio_t *)gpio_pin_struct[pinNumber];
	}
	pinTable[pinNumber].mode = pinMode;

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
	if (pinInvalid(pinNumber))
		return;
	if (pinTable[pinNumber].types != PIN_GPIO)
		return;

	gpio_t *gpio = (gpio_t *)gpio_pin_struct[pinNumber];
	gpio_write(gpio, status);
}

PinStatus digitalRead(pin_size_t pinNumber) {
	if (pinInvalid(pinNumber))
		return;
	if (pinTable[pinNumber].types != PIN_GPIO)
		return;

	gpio_t *gpio = (gpio_t *)gpio_pin_struct[pinNumber];
	return gpio_read(gpio);
}

/**************************** Extend API by RTK ***********************************/

uint32_t digitalPinToPort(uint32_t pinNumber) {
	if (pinInvalid(pinNumber))
		return 0xFFFFFFFF;

	uint32_t pin_name = HAL_GPIO_GetPinName(pinTable[pinNumber].gpio);
	return HAL_GPIO_GET_PORT_BY_NAME(pin_name);
}

uint32_t digitalPinToBitMask(uint32_t pinNumber) {
	if (pinInvalid(pinNumber))
		return 0xFFFFFFFF;

	uint32_t pin_name = HAL_GPIO_GetPinName(pinTable[pinNumber].gpio);
	return 1 << (HAL_GPIO_GET_PIN_BY_NAME(pin_name));
}

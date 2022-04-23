#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "gpio_api.h"
#include "gpio_irq_api.h"
#include "gpio_irq_ex_api.h"
#include "pwmout_api.h"

extern void *gpio_pin_struct[PINS_COUNT];
extern void *gpio_irq_handler_list[PINS_COUNT];

bool pinInvalid(pin_size_t pinNumber) {
	return pinNumber < 0 || pinNumber > PINS_COUNT || g_APinDescription[pinNumber].pinname == NC;
}

void pinRemoveMode(pin_size_t pinNumber) {
	if (pinInvalid(pinNumber))
		return;
	if (g_APinDescription[pinNumber].ulPinType == PIO_PWM) {
		pwmout_t *obj = (pwmout_t *)gpio_pin_struct[pinNumber];
		pwmout_free(obj);
	}
	if (g_APinDescription[pinNumber].ulPinType == PIO_GPIO) {
		gpio_t *obj = (gpio_t *)gpio_pin_struct[pinNumber];
		gpio_deinit(obj, g_APinDescription[pinNumber].pinname);
		free(obj);
	}
	if (g_APinDescription[pinNumber].ulPinType == PIO_GPIO_IRQ) {
		gpio_irq_t *obj = (gpio_irq_t *)gpio_pin_struct[pinNumber];
		gpio_irq_deinit(obj);
		free(obj);
	}
	gpio_pin_struct[pinNumber] = NULL;
	g_APinDescription[pinNumber].ulPinType = NOT_INITIAL;
	g_APinDescription[pinNumber].ulPinMode = NOT_INITIAL;
}

void pinMode(pin_size_t pinNumber, PinModeArduino pinMode) {
	if (pinInvalid(pinNumber))
		return;

	if (g_APinDescription[pinNumber].ulPinType == PIO_GPIO && g_APinDescription[pinNumber].ulPinMode == pinMode)
		// Nothing changes in pin mode
		return;

	/* if (g_APinDescription[pinNumber].ulPinType == PIO_PWM) {
		// If this pin has been configured as PWM, then it cannot change to another mode
		return;
	} */

	if (g_APinDescription[pinNumber].ulPinType != PIO_GPIO)
		// pin mode changes; deinit gpio and free memory
		pinRemoveMode(pinNumber);

	gpio_t *gpio;

	if (g_APinDescription[pinNumber].ulPinType == NOT_INITIAL) {
		// allocate memory if pin not used before
		gpio = malloc(sizeof(gpio_t));
		gpio_pin_struct[pinNumber] = gpio;
		gpio_init(gpio, g_APinDescription[pinNumber].pinname);
		g_APinDescription[pinNumber].ulPinType = PIO_GPIO;
	} else {
		// pin already used as gpio
		gpio = (gpio_t *)gpio_pin_struct[pinNumber];
	}
	g_APinDescription[pinNumber].ulPinMode = pinMode;

	PinDirection dir;
	PinMode mode;

	switch (pinMode) {
		case INPUT:
			dir = PIN_INPUT;
			mode = PullNone;
			break;
		case INPUT_PULLDOWN:
			dir = PIN_INPUT;
			mode = PullDown;
			break;
		case INPUT_PULLUP:
			dir = PIN_INPUT;
			mode = PullUp;
			break;
		case OUTPUT:
			dir = PIN_OUTPUT;
			mode = PullNone;
			break;
		case OUTPUT_OPENDRAIN:
			dir = PIN_OUTPUT;
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
	if (g_APinDescription[pinNumber].ulPinType != PIO_GPIO)
		return;

	gpio_t *gpio = (gpio_t *)gpio_pin_struct[pinNumber];
	gpio_write(gpio, status);
}

PinStatus digitalRead(pin_size_t pinNumber) {
	if (pinInvalid(pinNumber))
		return;
	if (g_APinDescription[pinNumber].ulPinType != PIO_GPIO)
		return;

	gpio_t *gpio = (gpio_t *)gpio_pin_struct[pinNumber];
	return gpio_read(gpio);
}

/**************************** Extend API by RTK ***********************************/

uint32_t digitalPinToPort(uint32_t pinNumber) {
	if (pinInvalid(pinNumber))
		return 0xFFFFFFFF;

	uint32_t pin_name = HAL_GPIO_GetPinName(g_APinDescription[pinNumber].pinname);
	return HAL_GPIO_GET_PORT_BY_NAME(pin_name);
}

uint32_t digitalPinToBitMask(uint32_t pinNumber) {
	if (pinInvalid(pinNumber))
		return 0xFFFFFFFF;

	uint32_t pin_name = HAL_GPIO_GetPinName(g_APinDescription[pinNumber].pinname);
	return 1 << (HAL_GPIO_GET_PIN_BY_NAME(pin_name));
}

#ifdef __cplusplus
}
#endif

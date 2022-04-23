#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "gpio_api.h"
#include "gpio_irq_api.h"
#include "gpio_irq_ex_api.h"

extern void *gpio_pin_struct[PINS_COUNT];
extern void *gpio_irq_handler_list[PINS_COUNT];

void gpioIrqHandler(uint32_t id, gpio_irq_event event) {
	if (gpio_irq_handler_list[id] != NULL) {
		((void (*)(uint32_t, uint32_t))gpio_irq_handler_list[id])(id, (uint32_t)event);
	}
}

void attachInterrupt(pin_size_t interruptNumber, voidFuncPtr callback, PinStatus mode) {
	if (pinInvalid(interruptNumber))
		return;

	gpio_irq_handler_list[interruptNumber] = callback;

	if (g_APinDescription[interruptNumber].ulPinType == PIO_GPIO_IRQ &&
		g_APinDescription[interruptNumber].ulPinMode == mode)
		// Nothing changes in pin mode
		return;

	if (g_APinDescription[interruptNumber].ulPinType != PIO_GPIO_IRQ)
		// pin mode changes; deinit gpio and free memory
		pinRemoveMode(interruptNumber);

	gpio_irq_t *gpio;

	if (g_APinDescription[interruptNumber].ulPinType == NOT_INITIAL) {
		// allocate memory if pin not used before
		gpio = malloc(sizeof(gpio_irq_t));
		gpio_pin_struct[interruptNumber] = gpio;
		gpio_irq_init(gpio, g_APinDescription[interruptNumber].pinname, gpioIrqHandler, interruptNumber);
		g_APinDescription[interruptNumber].ulPinType = PIO_GPIO_IRQ;
	} else {
		// pin already used as irq
		gpio = (gpio_irq_t *)gpio_pin_struct[interruptNumber];
	}
	g_APinDescription[interruptNumber].ulPinMode = mode;

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
		default:
			return;
	}

	gpio_irq_set(gpio, event, 1);
	gpio_irq_enable(gpio);
}

void detachInterrupt(pin_size_t interruptNumber) {
	if (pinInvalid(interruptNumber))
		return;

	if (g_APinDescription[interruptNumber].ulPinType == PIO_GPIO_IRQ) {
		pinRemoveMode(interruptNumber);
	}
	gpio_irq_handler_list[interruptNumber] = NULL;
}

#ifdef __cplusplus
}
#endif

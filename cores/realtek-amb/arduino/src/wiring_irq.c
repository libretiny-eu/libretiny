#include <Arduino.h>
#include <sdk_private.h>

extern void *gpio_pin_struct[PINS_COUNT];
static void *gpio_irq_handler_list[PINS_COUNT] = {NULL};
static void *gpio_irq_handler_args[PINS_COUNT] = {NULL};

extern bool pinInvalid(pin_size_t pinNumber);
extern void pinRemoveMode(pin_size_t pinNumber);

static void gpioIrqHandler(uint32_t id, gpio_irq_event event) {
	if (gpio_irq_handler_list[id] != NULL) {
		if (gpio_irq_handler_args[id] == NULL)
			((voidFuncPtr)gpio_irq_handler_list[id])();
		else
			((voidFuncPtrParam)gpio_irq_handler_list[id])(gpio_irq_handler_args[id]);
	}
}

void attachInterrupt(pin_size_t interruptNumber, voidFuncPtr callback, PinStatus mode) {
	attachInterruptParam(interruptNumber, callback, mode, NULL);
}

void attachInterruptParam(pin_size_t interruptNumber, voidFuncPtrParam callback, PinStatus mode, void *param) {
	if (pinInvalid(interruptNumber))
		return;

	gpio_irq_handler_list[interruptNumber] = callback;
	gpio_irq_handler_args[interruptNumber] = param;

	if (pinTable[interruptNumber].enabled == PIN_IRQ && pinTable[interruptNumber].mode == mode)
		// Nothing changes in pin mode
		return;

	if (pinTable[interruptNumber].enabled != PIN_IRQ)
		// pin mode changes; deinit gpio and free memory
		pinRemoveMode(interruptNumber);

	gpio_irq_t *gpio;

	if (pinTable[interruptNumber].enabled == PIN_NONE) {
		// allocate memory if pin not used before
		gpio							 = malloc(sizeof(gpio_irq_t));
		gpio_pin_struct[interruptNumber] = gpio;
		gpio_irq_init(gpio, pinTable[interruptNumber].gpio, gpioIrqHandler, interruptNumber);
		pinTable[interruptNumber].enabled = PIN_IRQ;
	} else {
		// pin already used as irq
		gpio = (gpio_irq_t *)gpio_pin_struct[interruptNumber];
	}
	pinTable[interruptNumber].mode = mode;

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

	if (pinTable[interruptNumber].enabled == PIN_IRQ) {
		pinRemoveMode(interruptNumber);
	}
	gpio_irq_handler_list[interruptNumber] = NULL;
}

#include <Arduino.h>
#include <sdk_private.h>

extern void *gpio_pin_struct[PINS_COUNT];
static void *gpio_irq_handler_list[PINS_COUNT] = {NULL};
static void *gpio_irq_handler_args[PINS_COUNT] = {NULL};

extern void pinRemoveMode(pin_size_t pinNumber);

static void gpioIrqHandler(uint32_t id, gpio_irq_event event) {
	// id is pin index
	if (gpio_irq_handler_list[id] != NULL) {
		if (gpio_irq_handler_args[id] == NULL)
			((voidFuncPtr)gpio_irq_handler_list[id])();
		else
			((voidFuncPtrParam)gpio_irq_handler_list[id])(gpio_irq_handler_args[id]);
	}
}

void attachInterrupt(pin_size_t interruptNumber, voidFuncPtr callback, PinStatus mode) {
	attachInterruptParam(interruptNumber, (voidFuncPtrParam)callback, mode, NULL);
}

void attachInterruptParam(pin_size_t interruptNumber, voidFuncPtrParam callback, PinStatus mode, void *param) {
	PinInfo *pin = pinInfo(interruptNumber);
	if (pin == NULL)
		return;
	uint32_t index = pinIndex(pin);

	gpio_irq_handler_list[index] = callback;
	gpio_irq_handler_args[index] = param;

	if (pin->enabled == PIN_IRQ && pin->mode == mode)
		// Nothing changes in pin mode
		return;

	if (pin->enabled != PIN_IRQ)
		// pin mode changes; deinit gpio and free memory
		pinRemoveMode(interruptNumber);

	gpio_irq_t *gpio;

	if (pin->enabled == PIN_NONE) {
		// allocate memory if pin not used before
		gpio				   = malloc(sizeof(gpio_irq_t));
		gpio_pin_struct[index] = gpio;
		gpio_irq_init(gpio, pin->gpio, gpioIrqHandler, index);
		pin->enabled = PIN_IRQ;
	} else {
		// pin already used as irq
		gpio = (gpio_irq_t *)gpio_pin_struct[index];
	}
	pin->mode = mode;

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
	PinInfo *pin = pinInfo(interruptNumber);
	if (pin == NULL)
		return;
	uint32_t index = pinIndex(pin);

	if (pin->enabled == PIN_IRQ) {
		pinRemoveMode(interruptNumber);
	}
	gpio_irq_handler_list[index] = NULL;
}

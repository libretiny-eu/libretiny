/* wiring_digital.c for silabs-efm32gg11.
 *
 * Phase 1 table-free implementation: each call decodes the (port<<4 | pin)
 * encoding produced by the variant header and calls emlib directly. We do not
 * use the common PinInfo/pinByGpio system in Phase 1 — Phase 2 may switch to
 * it once we need IRQ/PWM/etc. routing.
 */

#include "ArduinoFamily.h"
#include "em_gpio.h"
#include <Arduino.h>

void pinMode(pin_size_t pin, PinMode mode) {
	ltPwmDetach(pin); // reclaim the pin if it was under PWM
	GPIO_Mode_TypeDef gm;
	uint32_t out = 0U;
	switch (mode) {
		case INPUT:
			gm = gpioModeInput;
			break;
		case INPUT_PULLUP:
			gm	= gpioModeInputPull;
			out = 1U;
			break;
		case INPUT_PULLDOWN:
			gm	= gpioModeInputPull;
			out = 0U;
			break;
		case OUTPUT:
			gm = gpioModePushPull;
			break;
		case OUTPUT_OPENDRAIN:
			gm	= gpioModeWiredAnd;
			out = 1U;
			break;
		default:
			return; /* unsupported mode */
	}
	GPIO_PinModeSet(pin_port(pin), pin_index(pin), gm, out);
}

void digitalWrite(pin_size_t pin, PinStatus value) {
	ltPwmDetach(pin); // reclaim the pin if it was under PWM
	if (value == HIGH) {
		GPIO_PinOutSet(pin_port(pin), pin_index(pin));
	} else {
		GPIO_PinOutClear(pin_port(pin), pin_index(pin));
	}
}

PinStatus digitalRead(pin_size_t pin) {
	return GPIO_PinInGet(pin_port(pin), pin_index(pin)) ? HIGH : LOW;
}

/* ArduinoFamily.h for silabs-efm32gg11.
 *
 * Included by cores/common/arduino/src/Arduino.h to pull in family-specific
 * pin macros and helpers. Our boardgen variant header (PA4=0x04, PE7=0x47, ...)
 * encodes each pin as (port_index << 4) | pin_number. The inline helpers below
 * decode that back into emlib GPIO_Port_TypeDef and pin index for wiring_*.c.
 */

#pragma once

#include "em_gpio.h"
#include "lt_family.h"

/* Decode the (port << 4 | pin) encoding into emlib (GPIO_Port_TypeDef, pin).
 * Used by wiring_digital.c, wiring_irq.c (future), and the Serial library. */
static inline GPIO_Port_TypeDef pin_port(uint32_t pin) {
	return (GPIO_Port_TypeDef)(pin >> 4);
}

static inline uint32_t pin_index(uint32_t pin) {
	return pin & 0xFu;
}

/* PWM teardown hook (wiring_analog.c): if `pin` is currently driven by a TIMER
 * CC channel, disable its route so the pin reverts to plain GPIO. Returns true
 * if it was PWM. Called by wiring_digital.c on pinMode()/digitalWrite(). */
#include <api/Common.h> // pin_size_t — needed here because .c files include ArduinoFamily.h before Arduino.h
#ifdef __cplusplus
extern "C" {
#endif
bool ltPwmDetach(pin_size_t pin);
#ifdef __cplusplus
}
#endif

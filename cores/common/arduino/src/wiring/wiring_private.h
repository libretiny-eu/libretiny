/* Copyright (c) Kuba Szczodrzyński 2023-05-24. */

#pragma once

#include <Arduino.h>

#if __has_include(<sdk_private.h>)
#include <sdk_private.h>
#endif

#if __has_include(<wiring_data.h>)
#include <wiring_data.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

PinData *pinData(PinInfo *pin);
void pinRemoveData(PinInfo *pin);

inline void pinEnable(PinInfo *pin, uint32_t mask) {
	pin->enabled |= mask;
}

inline void pinDisable(PinInfo *pin, uint32_t mask) {
	pin->enabled &= ~mask;
}

#define pinCheckGetInfo(pinNumber, mask, ret) \
	PinInfo *pin = pinInfo(pinNumber);        \
	if (!pin)                                 \
		return ret;                           \
	if (!pinSupported(pin, mask))             \
		return ret;

#define pinCheckGetData(pinNumber, mask, ret) \
	PinInfo *pin = pinInfo(pinNumber);        \
	if (!pin)                                 \
		return ret;                           \
	if (!pinSupported(pin, mask))             \
		return ret;                           \
	PinData *data = pinData(pin);

#define pinIsOutput(pin, data) (pinEnabled(pin, PIN_GPIO) && (data->gpioMode ^ 0b101) < 5)
#define pinIsInput(pin, data)  (pinEnabled(pin, PIN_GPIO) && (data->gpioMode ^ 0b101) > 4)

#define pinSetOutputPull(pin, data, pinNumber, status)     \
	do {                                                   \
		if (!pinIsOutput(pin, data)) {                     \
			pinMode(pinNumber, INPUT_PULLDOWN ^ !!status); \
			return;                                        \
		}                                                  \
	} while (0);

#define pinSetInputMode(pin, data, pinNumber) \
	do {                                      \
		if (!pinIsInput(pin, data))           \
			pinMode(pinNumber, INPUT);        \
	} while (0);

#ifdef __cplusplus
} // extern "C"
#endif

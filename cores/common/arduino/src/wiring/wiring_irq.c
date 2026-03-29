/* Copyright (c) Kuba Szczodrzyński 2023-05-24. */

#include <Arduino.h>

void attachInterrupt(pin_size_t interruptNumber, voidFuncPtr callback, PinStatus mode) {
	attachInterruptParam(interruptNumber, (voidFuncPtrParam)callback, mode, NULL);
}

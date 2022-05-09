/* Copyright (c) Kuba Szczodrzyński 2022-04-23. */

#pragma once

#ifdef __cplusplus
#include "WCharacterFixup.h"
#endif

#define PinMode PinModeArduino // this conflicts with SDK enum
#include <api/ArduinoAPI.h>
#include <core/LibreTuyaAPI.h>
#undef PinMode

#ifdef __cplusplus
extern "C" uint32_t SystemCoreClock;
#else
extern uint32_t SystemCoreClock;
#endif
#define clockCyclesPerMicrosecond()	 (SystemCoreClock / 1000000L)
#define clockCyclesToMicroseconds(a) (((a)*1000L) / (SystemCoreClock / 1000L))
#define microsecondsToClockCycles(a) ((a) * (SystemCoreClock / 1000000L))

#define interrupts()   vPortClearInterruptMask(0)
#define noInterrupts() ulPortSetInterruptMask()

// Include platform-specific code
#include "WVariant.h"
// Include board variant
#include "variant.h"

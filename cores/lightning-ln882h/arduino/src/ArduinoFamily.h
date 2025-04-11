/* Copyright (c) Etienne Le Cousin 2024-02-10. */

#pragma once

// Provide GPIO names to variant.cpp files
#define LT_VARIANT_INCLUDE "sdk_private.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern void vPortClearInterruptMask(uint32_t ulNewMaskValue);
extern uint32_t ulPortSetInterruptMask(void);

// TODO
// #define clockCyclesPerMicrosecond()	 (SystemCoreClock / 1000000L)
// #define clockCyclesToMicroseconds(a) (a * 1000L / (SystemCoreClock / 1000L))
// #define microsecondsToClockCycles(a) (a * (SystemCoreClock / 1000000L))

#define interrupts()   vPortClearInterruptMask(0)
#define noInterrupts() ulPortSetInterruptMask()

#ifdef __cplusplus
} // extern "C"
#endif

/* Copyright (c) Kuba Szczodrzyński 2023-03-02. */

#pragma once

// Provide GPIO names to variant.cpp files
#define LT_VARIANT_INCLUDE "gpio_pub.h"

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

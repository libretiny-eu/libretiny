#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sdk_extern.h"
#include "sdk_mem.h"
#include "sdk_os.h"

#define DEFAULT	 1
#define EXTERNAL 0

#define round(x) ((x) >= 0 ? (long)((x) + 0.5) : (long)((x)-0.5))

// Additional Wiring functions
extern uint32_t digitalPinToPort(uint32_t pinNumber);
extern uint32_t digitalPinToBitMask(uint32_t pinNumber);
extern void analogOutputInit(void);
extern void wait_for_debug();

#ifdef __cplusplus
extern "C" uint32_t SystemCoreClock;
#else
extern uint32_t SystemCoreClock;
#endif
#define clockCyclesPerMicrosecond()	 (SystemCoreClock / 1000000L)
#define clockCyclesToMicroseconds(a) (a * 1000L / (SystemCoreClock / 1000L))
#define microsecondsToClockCycles(a) (a * (SystemCoreClock / 1000000L))

#define interrupts()   vPortClearInterruptMask(0)
#define noInterrupts() ulPortSetInterruptMask()

#ifdef __cplusplus
} // extern "C"
#endif

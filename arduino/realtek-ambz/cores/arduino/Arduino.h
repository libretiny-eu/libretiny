/*
  Arduino.h - Main include file for the Arduino SDK
  Copyright (c) 2005-2013 Arduino Team.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#ifdef __cplusplus
#include "WCharacterFixup.h"
#endif
#define PinMode PinModeArduino // this conflicts with SDK enum
#include <api/ArduinoAPI.h>
#include <api/LibreTuyaAPI.h>
#undef PinMode

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern uint32_t SystemCoreClock;

#define clockCyclesPerMicrosecond() ( SystemCoreClock / 1000000L )
#define clockCyclesToMicroseconds(a) ( ((a) * 1000L) / (SystemCoreClock / 1000L) )
#define microsecondsToClockCycles(a) ( (a) * (SystemCoreClock / 1000000L) )

#include "WVariant.h"

#ifdef __cplusplus
} // extern "C"
#endif

// Include board variant
#include "variant.h"

#define interrupts() vPortClearInterruptMask(0)
#define noInterrupts() ulPortSetInterruptMask()

/*
 * \brief SAM3 products have only one reference for ADC
 */
typedef enum _eAnalogReference
{
  AR_DEFAULT,
} eAnalogReference ;

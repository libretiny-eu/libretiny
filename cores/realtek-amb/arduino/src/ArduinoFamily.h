/* Copyright (c) Kuba Szczodrzyński 2022-04-23. */

#pragma once

#ifdef __cplusplus
#include "WCharacterFixup.h"
#endif

#define PinMode PinModeArduino // this conflicts with SDK enum
#include <api/ArduinoAPI.h>
#include <core/LibreTuyaAPI.h>
#undef PinMode

// Include family-specific code
#include "WVariant.h"
// Include board variant
#include "variant.h"

// Choose the main UART output port
#ifndef LT_UART_DEFAULT_PORT
#if HAS_SERIAL2
#define LT_UART_DEFAULT_PORT 2
#elif HAS_SERIAL0
#define LT_UART_DEFAULT_PORT 0
#elif HAS_SERIAL1
#define LT_UART_DEFAULT_PORT 1
#else
#error "No serial port is available"
#endif
#endif

// Define available serial ports
#ifdef __cplusplus
#include "SerialClass.h"
#include <core/SerialExtern.h>
#endif

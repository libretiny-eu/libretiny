/* Copyright (c) Kuba Szczodrzyński 2022-06-14. */

#pragma once

// LibreTuya C API (with C standard libraries)
#include <libretuya.h>

// C++ standard libraries
#ifdef __cplusplus
#include <algorithm>
#include <cmath>
using ::round;
using std::abs;
using std::isinf;
using std::isnan;
using std::max;
using std::min;
#endif

// Arduino Core and LT class
#include <api/ArduinoAPI.h>
#ifdef __cplusplus
#include <LT.h>
#endif

// Include family-specific code
#include <ArduinoFamily.h>

// Additional Wiring headers
#include "wiring_compat.h"
#include "wiring_custom.h"

// Define available serial ports
#if defined(__cplusplus) && LT_ARD_HAS_SERIAL
#include <Serial.h>

#if HAS_SERIAL0
extern SerialClass Serial0;
#endif
#if HAS_SERIAL1
extern SerialClass Serial1;
#endif
#if HAS_SERIAL2
extern SerialClass Serial2;
#endif

#define SerialN(x) Serial##x
#define SerialM(x) SerialN(x)
#define Serial	   SerialM(LT_UART_DEFAULT_SERIAL)
#endif

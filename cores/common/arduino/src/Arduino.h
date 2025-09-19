/* Copyright (c) Kuba Szczodrzyński 2022-06-14. */

#pragma once

// LibreTiny C API (with C standard libraries)
#include <libretiny.h>

// Additional C libraries
#include <sys/time.h>
#include <time.h>

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
using namespace arduino;
#endif

// Include family-specific code
#include <ArduinoFamily.h>

// Additional Wiring headers
#include "wiring_compat.h"
#include "wiring_custom.h"

// FreeRTOS kernel
#include <FreeRTOS.h>
#include <task.h>

// Define available serial ports
#if defined(__cplusplus) && LT_ARD_HAS_SERIAL
#include <Serial.h>

#if HAS_SERIAL_CLASS
#if LT_HW_UART0
extern SerialClass Serial0;
#endif
#if LT_HW_UART1
extern SerialClass Serial1;
#endif
#if LT_HW_UART2
extern SerialClass Serial2;
#endif
#endif

#define SerialN(x) Serial##x
#define SerialM(x) SerialN(x)
#define Serial	   SerialM(LT_UART_DEFAULT_SERIAL)
#endif

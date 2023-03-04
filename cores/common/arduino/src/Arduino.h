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
// Include board variant
#include "variant.h"

/**
 * @brief Run mainTask & start OS kernel (family-defined).
 * Return false if an error occured; else do not return and
 * and keep the OS kernel running.
 */
extern int startMainTask(void);

// Define available serial ports
#if defined(__cplusplus) && LT_ARD_HAS_SERIAL
#include <SerialClass.h>

#ifdef PIN_SERIAL0_TX
extern SerialClass Serial0;
#endif

#ifdef PIN_SERIAL1_TX
extern SerialClass Serial1;
#endif

#ifdef PIN_SERIAL2_TX
extern SerialClass Serial2;
#endif

#define SerialN(x) Serial##x
#define SerialM(x) SerialN(x)
#define Serial	   SerialM(LT_UART_DEFAULT_SERIAL)
#endif

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
#include <api/HardwareSerial.h>
#include <api/RingBuffer.h>
using namespace arduino;
#include <LT.h>
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
#endif

/* Copyright (c) Kuba Szczodrzyński 2022-06-14. */

#pragma once

#ifdef __cplusplus
#include "WCharacterFixup.h"
#endif

#include <api/ArduinoAPI.h>
#include <core/LibreTuyaAPI.h>

// Include family-specific code
#include "WVariant.h"
// Include board variant
#include "variant.h"

#ifdef __cplusplus
#include "SerialClass.h"
#ifdef HAS_SERIAL_CLASS
// declare instances of available Serial* components
// map Serial to Serial2 if available, else to Serial1
#ifdef PIN_SERIAL1_RX
extern SerialClass Serial1;
#endif
#ifdef PIN_SERIAL2_RX
extern SerialClass Serial2;
#define Serial Serial2
#else
#define Serial Serial1
#endif
#endif
#endif

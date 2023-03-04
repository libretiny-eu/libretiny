/* Copyright (c) Kuba Szczodrzyński 2022-06-14. */

#pragma once

#ifdef __cplusplus
#include "WCharacterFixup.h"
#endif

#define delay delayMilliseconds // change delay()'s signature - it's defined as static inline in WVariant.h
#include <api/ArduinoAPI.h>
#include <core/LibreTuyaAPI.h>
#undef delay

// Include family-specific code
#include "WVariant.h"

// Define available serial ports
#ifdef __cplusplus
#include "SerialClass.h"
#include <core/SerialExtern.h>
#endif

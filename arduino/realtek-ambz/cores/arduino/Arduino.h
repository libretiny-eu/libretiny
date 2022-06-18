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

#ifdef __cplusplus
#include "LOGUARTClass.h"
extern LOGUARTClass Serial;
#endif

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

// Define available serial ports
#ifdef __cplusplus
#include "SerialClass.h"
#ifdef HAS_SERIAL_CLASS // failsafe for circular inclusion

#ifdef PIN_SERIAL0_TX
extern SerialClass Serial0;
#endif

#ifdef PIN_SERIAL1_TX
extern SerialClass Serial1;
#endif

#ifdef PIN_SERIAL2_RX
extern SerialClass Serial2;
#define Serial Serial2
#elif PIN_SERIAL0_TX
#define Serial Serial0
#else
#define Serial Serial1
#endif

#endif
#endif

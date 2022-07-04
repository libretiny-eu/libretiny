/* Copyright (c) Kuba Szczodrzyński 2022-07-04. */

#pragma once

#include <Arduino.h>

#ifdef HAS_SERIAL_CLASS // failsafe for circular inclusion

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

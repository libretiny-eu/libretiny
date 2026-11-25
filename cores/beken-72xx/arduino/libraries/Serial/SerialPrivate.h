/* Copyright (c) Kuba Szczodrzyński 2023-05-24. */

#pragma once

#include <ArduinoPrivate.h>

struct SerialData {
	bool started; //!< uart1_init()/uart2_init() was called, and not undone by endPrivate()
};

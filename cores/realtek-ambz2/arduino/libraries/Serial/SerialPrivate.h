/* Copyright (c) Kuba Szczodrzyński 2023-05-24. */

#pragma once

#include <ArduinoPrivate.h>
#include <Serial.h>

struct SerialData {
	SerialRingBuffer *buf;
	hal_uart_adapter_t *uart;
};

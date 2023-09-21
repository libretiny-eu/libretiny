/* Copyright (c) Kuba Szczodrzyński 2023-05-24. */

#pragma once

#include <Arduino.h>
#include <sdk_private.h>

struct SerialData {
	hal_uart_adapter_t *uart;
	SerialRingBuffer buf;
};

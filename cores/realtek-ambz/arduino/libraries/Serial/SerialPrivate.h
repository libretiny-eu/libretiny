/* Copyright (c) Kuba Szczodrzyński 2023-05-23. */

#pragma once

#include <Arduino.h>
#include <sdk_private.h>

struct SerialData {
	UART_TypeDef *uart;
	IRQn irq;
	SerialRingBuffer buf;
};

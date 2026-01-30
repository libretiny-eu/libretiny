/* Copyright (c) Kuba Szczodrzyński 2023-05-23. */

#pragma once

#include <ArduinoPrivate.h>
#include <Serial.h>

struct SerialData {
	UART_TypeDef *uart;
	IRQn irq;
	SerialRingBuffer buf;
};

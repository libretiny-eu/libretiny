/* Copyright (c) Kuba Szczodrzyński 2023-05-23. */

#pragma once

#include <ArduinoPrivate.h>
#include <Serial.h>

struct SerialData {
	SerialRingBuffer *buf{nullptr};
	UART_TypeDef *uart{nullptr};
	IRQn irq;
};

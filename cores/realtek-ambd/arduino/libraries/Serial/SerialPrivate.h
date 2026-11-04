/* Copyright (c) Bl00d-B0b 2026-08-07. */

#pragma once

#include <ArduinoPrivate.h>
#include <Serial.h>

struct SerialData {
	SerialRingBuffer *buf{nullptr};
	UART_TypeDef *uart{nullptr};
	IRQn_Type irq;
};

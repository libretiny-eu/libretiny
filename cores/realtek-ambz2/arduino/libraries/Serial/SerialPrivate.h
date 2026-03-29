/* Copyright (c) Kuba Szczodrzyński 2023-05-24. */

#pragma once

#include <ArduinoPrivate.h>
#include <Serial.h>

struct SerialData {
	uint32_t port{0};
	SerialRingBuffer *buf{nullptr};
	serial_t *uart{nullptr};
};

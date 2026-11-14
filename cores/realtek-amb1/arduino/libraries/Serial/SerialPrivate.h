#pragma once
#include <ArduinoPrivate.h>
#include <Serial.h>

struct SerialData {
	SerialRingBuffer *buf{nullptr};
	serial_t uart{};
	bool initialized{false};
};

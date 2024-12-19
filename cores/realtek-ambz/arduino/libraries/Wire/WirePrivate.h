/* Copyright (c) Kuba Szczodrzyński 2023-09-21. */

#pragma once

#include <ArduinoPrivate.h>
#include <Wire.h>

struct WireData {
	RingBuffer *buf;
	I2C_TypeDef *i2c;
	I2C_InitTypeDef init;
};

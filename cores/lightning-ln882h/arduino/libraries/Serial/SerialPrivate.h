/* Copyright (c) Etienne Le Cousin 2024-02-10. */

#pragma once

#include <Arduino.h>
#include <sdk_private.h>

typedef struct {
	SerialRingBuffer buf;
	void (*callback)(void);
} SerialData;

#define DATA ((SerialData *)this->data)
#define BUF	 (DATA->buf)

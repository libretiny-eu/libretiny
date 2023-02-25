/* Copyright (c) Kuba Szczodrzyński 2022-06-23. */

#pragma once

#include <Arduino.h>

extern "C" {

#include <FreeRTOS.h>
#include <semphr.h>

} // extern "C"

typedef struct {
	bool initialized;
	bool sleep;
	SemaphoreHandle_t scanSem;
} WiFiData;

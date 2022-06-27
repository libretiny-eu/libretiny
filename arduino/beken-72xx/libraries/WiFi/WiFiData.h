/* Copyright (c) Kuba Szczodrzyński 2022-06-26. */

#pragma once

#include <Arduino.h>

extern "C" {

#include <FreeRTOS.h>
#include <semphr.h>

} // extern "C"

typedef struct {
	SemaphoreHandle_t scanSem;
} WiFiData;

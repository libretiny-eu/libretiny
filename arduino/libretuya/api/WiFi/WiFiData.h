/* Copyright (c) Kuba Szczodrzyński 2022-06-23. */

#pragma once

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <FreeRTOS.h>
#include <semphr.h>

#ifdef __cplusplus
} // extern "C"
#endif

typedef struct {
	bool initialized;
	bool sleep;
	SemaphoreHandle_t scanSem;
} WiFiData;

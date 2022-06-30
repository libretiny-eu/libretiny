/* Copyright (c) Kuba Szczodrzyński 2022-06-26. */

#pragma once

#include <Arduino.h>

extern "C" {

#include <FreeRTOS.h>
#include <semphr.h>

} // extern "C"

typedef struct {
	char ssid[32 + 1];
	char pass[64 + 1];
	unsigned long scannedAt;
	uint32_t ipSta[4];
	uint32_t ipAp[4];
	SemaphoreHandle_t scanSem;
	void *statusIp;
	void *statusLink;
} WiFiData;

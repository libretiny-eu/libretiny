/* Copyright (c) Kuba Szczodrzyński 2022-06-26. */

#pragma once

#include <Arduino.h>

extern "C" {

#include <FreeRTOS.h>
#include <rw_msg_pub.h>
#include <semphr.h>

} // extern "C"

typedef struct {
	void *configSta;
	void *configAp;
	unsigned long scannedAt;
	SemaphoreHandle_t scanSem;
	void *statusIp;
	void *statusLink;
	rw_evt_type lastStaEvent;
	rw_evt_type lastApEvent;
	bool apEnabled;
} WiFiData;

/* Copyright (c) Kuba Szczodrzyński 2022-06-26. */

#pragma once

#include <Arduino.h>

extern "C" {
#define _ARCH_H_
#define _GENERIC_H_
#include <FreeRTOS.h>
#include <include.h>
#include <rw_msg_pub.h>
#include <semphr.h>
#undef _ARCH_H_
#undef _GENERIC_H_
} // extern "C"

typedef struct {
	void *configSta;
	void *configAp;
	unsigned long scannedAt;
	SemaphoreHandle_t scanSem;
	void *statusIp;
	void *statusLink;
	uint32_t lastStaEvent; // TODO revert this type back to rw_evt_type
	uint32_t lastApEvent;
	bool apEnabled;
} WiFiData;

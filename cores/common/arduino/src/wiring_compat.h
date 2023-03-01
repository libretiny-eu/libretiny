/* Copyright (c) Kuba Szczodrzyński 2022-06-04. */

#pragma once

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#if LT_HAS_FREERTOS
// dirty hack to avoid including BDK's arch.h (from FreeRTOS/portable.h)
#define _ARCH_H_
#include <FreeRTOS.h>
#include <task.h>
#undef _ARCH_H_
#endif

// Definitions for error constants.
#define esp_err_t int
#define ESP_OK	  0	 /*!< esp_err_t value indicating success (no error) */
#define ESP_FAIL  -1 /*!< Generic esp_err_t code indicating failure */

// ArduinoCore-API doesn't define these anymore
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#define PGM_VOID_P			const void *
#define vsnprintf_P			vsnprintf
#define OUTPUT_OPEN_DRAIN	OUTPUT_OPENDRAIN
#define attachInterruptArg	attachInterruptParam
#define voidFuncPtrArg		voidFuncPtrParam

// FreeRTOS utilities
#if LT_HAS_FREERTOS
// if xCoreID < 0 or CPU is unicore, it will use xTaskCreate, else xTaskCreatePinnedToCore
// allows to easily handle all possible situations without repetitive code
BaseType_t xTaskCreateUniversal(
	TaskFunction_t pxTaskCode,
	const char *const pcName,
	const uint32_t usStackDepth,
	void *const pvParameters,
	UBaseType_t uxPriority,
	TaskHandle_t *const pxCreatedTask,
	const BaseType_t xCoreID
);
#define xTaskCreatePinnedToCore xTaskCreateUniversal
#endif

// Default values from sdkconfig.h
#define CONFIG_LWIP_MAX_ACTIVE_TCP 16

#ifdef __cplusplus
String ipToString(const IPAddress &ip);
#endif

#ifdef __cplusplus
} // extern "C"
#endif

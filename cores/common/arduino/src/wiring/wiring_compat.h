/* Copyright (c) Kuba Szczodrzyński 2022-06-04. */

#pragma once

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

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

// Additional Arduino compatibility macros
#define round(x)				   ((x) >= 0 ? (long)((x) + 0.5) : (long)((x)-0.5))
#define digitalPinToInterrupt(pin) (pin)

// FreeRTOS utilities
#define xTaskCreateUniversal(pxTaskCode, pcName, usStackDepth, pvParameters, uxPriority, pxCreatedTask, xCoreID)       \
	xTaskCreate(pxTaskCode, pcName, usStackDepth, pvParameters, uxPriority, pxCreatedTask)
#define xTaskCreatePinnedToCore xTaskCreateUniversal

// Default values from sdkconfig.h
#define CONFIG_LWIP_MAX_ACTIVE_TCP 16

#ifdef __cplusplus
String ipToString(const IPAddress &ip);
#endif

#ifdef __cplusplus
} // extern "C"
#endif

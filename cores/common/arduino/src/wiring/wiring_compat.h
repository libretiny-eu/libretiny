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

#define ESP_ERR_NO_MEM              0x101   /*!< Out of memory */
#define ESP_ERR_INVALID_ARG         0x102   /*!< Invalid argument */
#define ESP_ERR_INVALID_STATE       0x103   /*!< Invalid state */
#define ESP_ERR_INVALID_SIZE        0x104   /*!< Invalid size */
#define ESP_ERR_NOT_FOUND           0x105   /*!< Requested resource not found */
#define ESP_ERR_NOT_SUPPORTED       0x106   /*!< Operation or feature not supported */
#define ESP_ERR_TIMEOUT             0x107   /*!< Operation timed out */
#define ESP_ERR_INVALID_RESPONSE    0x108   /*!< Received response was invalid */
#define ESP_ERR_INVALID_CRC         0x109   /*!< CRC or checksum was invalid */
#define ESP_ERR_INVALID_VERSION     0x10A   /*!< Version was invalid */
#define ESP_ERR_INVALID_MAC         0x10B   /*!< MAC address was invalid */
#define ESP_ERR_NOT_FINISHED        0x10C   /*!< Operation has not fully completed */
#define ESP_ERR_NOT_ALLOWED         0x10D   /*!< Operation is not allowed */


#define ESP_ERR_WIFI_BASE           0x3000  /*!< Starting number of WiFi error codes */
#define ESP_ERR_MESH_BASE           0x4000  /*!< Starting number of MESH error codes */
#define ESP_ERR_FLASH_BASE          0x6000  /*!< Starting number of flash error codes */
#define ESP_ERR_HW_CRYPTO_BASE      0xc000  /*!< Starting number of HW cryptography module error codes */
#define ESP_ERR_MEMPROT_BASE        0xd000  /*!< Starting number of Memory Protection API error codes */

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

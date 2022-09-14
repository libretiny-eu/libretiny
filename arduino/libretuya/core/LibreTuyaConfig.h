/* Copyright (c) Kuba Szczodrzyński 2022-04-28. */

#pragma once

// see docs/API Configuration

// Loglevels
#define LT_LEVEL_VERBOSE LT_LEVEL_TRACE
#define LT_LEVEL_TRACE	 0
#define LT_LEVEL_DEBUG	 1
#define LT_LEVEL_INFO	 2
#define LT_LEVEL_WARN	 3
#define LT_LEVEL_ERROR	 4
#define LT_LEVEL_FATAL	 5
#define LT_LEVEL_NONE	 6

// Logger enabled/disabled
#ifndef LT_LOGGER
#define LT_LOGGER 1
#endif

// Logger format options
#ifndef LT_LOGGER_TIMESTAMP
#define LT_LOGGER_TIMESTAMP 1
#endif

#ifndef LT_LOGGER_CALLER
#define LT_LOGGER_CALLER 0
#endif

#ifndef LT_LOGGER_TASK
#define LT_LOGGER_TASK 0
#endif

#ifndef LT_LOGGER_COLOR
#define LT_LOGGER_COLOR 0
#endif

#ifndef LT_PRINTF_BROKEN
#define LT_PRINTF_BROKEN 0
#endif

// Global loglevel
#ifndef LT_LOGLEVEL
#define LT_LOGLEVEL LT_LEVEL_INFO
#endif

#if !LT_LOGGER
#undef LT_LOGLEVEL
#define LT_LOGLEVEL LT_LEVEL_NONE
#endif

// Free heap size debugging
#ifndef LT_LOG_HEAP
#define LT_LOG_HEAP 0
#endif

// Debug errno values using LT_ERRNO()
#ifndef LT_LOG_ERRNO
#define LT_LOG_ERRNO 0
#endif

// Serial output options
#ifndef LT_UART_SILENT_ENABLED
#define LT_UART_SILENT_ENABLED 1
#endif

#ifndef LT_UART_SILENT_ALL
#define LT_UART_SILENT_ALL 0
#endif

#ifndef LT_UART_DEFAULT_LOGGER
#define LT_UART_DEFAULT_LOGGER LT_UART_DEFAULT_PORT
#endif

#ifndef LT_UART_DEFAULT_SERIAL
#define LT_UART_DEFAULT_SERIAL LT_UART_DEFAULT_PORT
#endif

// Misc options
#ifndef LT_USE_TIME
#define LT_USE_TIME 0
#endif

#ifndef LT_MICROS_HIGH_RES // NOTE: this is also defined in fixups/clock_rtos.c
#define LT_MICROS_HIGH_RES 1
#endif

#ifndef LT_AUTO_DOWNLOAD_REBOOT
#define LT_AUTO_DOWNLOAD_REBOOT 1
#endif

// Per-module logging output - applies to all loglevels
#ifndef LT_DEBUG_ALL
#define LT_DEBUG_ALL 0
#endif

#ifndef LT_DEBUG_WIFI
#define LT_DEBUG_WIFI 1
#endif

#ifndef LT_DEBUG_CLIENT
#define LT_DEBUG_CLIENT LT_DEBUG_ALL
#endif

#ifndef LT_DEBUG_SERVER
#define LT_DEBUG_SERVER LT_DEBUG_ALL
#endif

#ifndef LT_DEBUG_SSL
#define LT_DEBUG_SSL LT_DEBUG_ALL
#endif

#ifndef LT_DEBUG_OTA
#define LT_DEBUG_OTA 1
#endif

#ifndef LT_DEBUG_FDB
#define LT_DEBUG_FDB 0
#endif

#ifndef LT_DEBUG_MDNS
#define LT_DEBUG_MDNS LT_DEBUG_ALL
#endif

#ifndef LT_DEBUG_LWIP
#define LT_DEBUG_LWIP 0
#endif

#ifndef LT_DEBUG_LWIP_ASSERT
#define LT_DEBUG_LWIP_ASSERT 0
#endif

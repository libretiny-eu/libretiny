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

// Logger enabled/disabled
#ifndef LT_LOGGER
#define LT_LOGGER 1
#endif

// Logger format options
#ifndef LT_LOGGER_TIMESTAMP
#define LT_LOGGER_TIMESTAMP 1
#endif

#ifndef LT_LOGGER_CALLER
#define LT_LOGGER_CALLER 1
#endif

#ifndef LT_LOGGER_TASK
#define LT_LOGGER_TASK 1
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

// Per-module debugging
#ifndef LT_DEBUG_WIFI
#define LT_DEBUG_WIFI 0
#endif

#ifndef LT_DEBUG_WIFI_CLIENT
#define LT_DEBUG_WIFI_CLIENT 0
#endif

#ifndef LT_DEBUG_WIFI_SERVER
#define LT_DEBUG_WIFI_SERVER 0
#endif

#ifndef LT_DEBUG_WIFI_STA
#define LT_DEBUG_WIFI_STA 0
#endif

#ifndef LT_DEBUG_WIFI_AP
#define LT_DEBUG_WIFI_AP 0
#endif

#ifndef LT_DEBUG_SSL
#define LT_DEBUG_SSL 0
#endif

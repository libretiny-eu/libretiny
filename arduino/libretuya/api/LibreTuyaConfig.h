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

#ifndef LT_LOGGER_FILE
#define LT_LOGGER_FILE 0
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

#pragma once

#error "Don't include this file directly"

#define LT_HAS_FLASH	 1
#define LT_HAS_FREERTOS	 1
#define LT_HAS_LWIP		 1
#define LT_HAS_LWIP2	 1
#define LT_HAS_MBEDTLS	 1
#define LT_HAS_OTA		 1
#define LT_HAS_PRINTF	 1
#define LT_HW_DEEP_SLEEP 0
#define LT_HW_WATCHDOG	 1
#define LT_HW_WIFI		 1

#define LT_REMALLOC 1

#define LT_HEAP_FUNC	xPortGetFreeHeapSize
#define LT_REALLOC_FUNC pvPortReAlloc

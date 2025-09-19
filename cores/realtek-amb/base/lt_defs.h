#pragma once

#error "Don't include this file directly"

#define LT_HAS_FLASH   1
#define LT_HW_WATCHDOG 1
#define LT_HW_WIFI	   1

#define LT_HEAP_FUNC	xPortGetFreeHeapSize
#define LT_REALLOC_FUNC pvPortReAlloc

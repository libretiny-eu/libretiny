#pragma once

#error "Don't include this file directly"

#define LT_HAS_PRINTF 1

#define LT_HEAP_FUNC	xPortGetFreeHeapSize
#define LT_REALLOC_FUNC pvPortReAlloc

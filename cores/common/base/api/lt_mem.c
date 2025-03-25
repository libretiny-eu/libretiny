/* Copyright (c) Kuba Szczodrzyński 2022-04-29. */

#include "lt_mem.h"

#if LT_HAS_FREERTOS
#include <FreeRTOS.h>
#include <task.h>
#endif

#if LT_HAS_FREERTOS
__attribute__((weak)) uint32_t lt_heap_get_size() {
	return configTOTAL_HEAP_SIZE;
}

__attribute__((weak)) uint32_t lt_heap_get_free() {
	return xPortGetFreeHeapSize();
}

__attribute__((weak)) uint32_t lt_heap_get_min_free() {
	return xPortGetMinimumEverFreeHeapSize();
}
#endif

__attribute__((weak)) uint32_t lt_heap_get_max_alloc() {
	return 0;
}

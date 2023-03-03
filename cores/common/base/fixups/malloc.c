/* Copyright (c) Kuba Szczodrzyński 2023-03-03. */

// Generic implementation of malloc() family wrappers for FreeRTOS

#if LT_HAS_FREERTOS

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <FreeRTOS.h>

// no such thing in FreeRTOS, but present on most vendor SDKs
extern void *LT_REALLOC_FUNC(void *pv, size_t xWantedSize);

void *__wrap_malloc(size_t size) {
	return pvPortMalloc(size);
}

void *__wrap_calloc(size_t num, size_t size) {
	void *ptr;
	if (num == 0 || size == 0)
		num = size = 1;
	ptr = pvPortMalloc(num * size);
	if (ptr)
		memset(ptr, 0, num * size);
	return ptr;
}

void *__wrap_realloc(void *ptr, size_t new_size) {
	return LT_REALLOC_FUNC(ptr, new_size);
}

void __wrap_free(void *ptr) {
	vPortFree(ptr);
}

void *__wrap__malloc_r(void *reent, size_t size) {
	return pvPortMalloc(size);
}

void *__wrap__calloc_r(void *reent, size_t num, size_t size) {
	void *ptr;
	if (num == 0 || size == 0)
		num = size = 1;
	ptr = pvPortMalloc(num * size);
	if (ptr)
		memset(ptr, 0, num * size);
	return ptr;
}

void *__wrap__realloc_r(void *reent, void *ptr, size_t new_size) {
	return LT_REALLOC_FUNC(ptr, new_size);
}

void __wrap__free_r(void *reent, void *ptr) {
	vPortFree(ptr);
}

#endif

// Additionally, define zalloc() as a shorthand to calloc() - some implementation use it

void *__wrap_zalloc(size_t size) {
	return __wrap_calloc(1, size);
}

__attribute__((alias("__wrap_zalloc"), weak)) void *zalloc(size_t size);

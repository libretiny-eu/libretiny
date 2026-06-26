/* Copyright (c) Kuba Szczodrzyński 2023-03-03. */

// Generic implementation of malloc() family wrappers for FreeRTOS

#if LT_HAS_FREERTOS

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <FreeRTOS.h>

// no such thing in FreeRTOS, but present on most vendor SDKs
extern void *LT_REALLOC_FUNC(void *pv, size_t xWantedSize);

#if LT_REMALLOC
// malloc+copy+free realloc fallback for SDKs without a native realloc.
// realloc(ptr, 0) must NOT reach pvPortMalloc(): heap_4 returns NULL for a
// 0-byte request and (with configUSE_MALLOC_FAILED_HOOK) fires the
// malloc-failed hook on ANY NULL return — on ports whose hook resets the SoC,
// a perfectly legal shrink-to-zero (e.g. Mongoose's mbuf_trim of an emptied
// buffer) becomes a reboot loop. Free and return NULL instead (C99
// implementation-defined, matches glibc). Also skip the copy when ptr is NULL
// (realloc-as-malloc): memcpy from NULL reads whatever sits at address 0.
static void *lt_remalloc(void *ptr, size_t new_size) {
	void *nptr;
	if (new_size == 0) {
		vPortFree(ptr);
		return NULL;
	}
	nptr = pvPortMalloc(new_size);
	if (nptr) {
		if (ptr)
			// NOTE: the old block size isn't known here; copying new_size
			// bytes over-reads the old block on grow (pre-existing behavior).
			memcpy(nptr, ptr, new_size);
		vPortFree(ptr);
	}
	return nptr;
}
#endif

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
#if LT_REMALLOC
	return lt_remalloc(ptr, new_size);
#else
	return LT_REALLOC_FUNC(ptr, new_size);
#endif
}

void __wrap_free(void *ptr) {
	vPortFree(ptr);
}

// Mind the 'reent' parameter - do NOT define these as linker aliases!

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
#if LT_REMALLOC
	return lt_remalloc(ptr, new_size);
#else
	return LT_REALLOC_FUNC(ptr, new_size);
#endif
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

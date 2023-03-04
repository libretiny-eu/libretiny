/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#pragma once

#include <libretuya.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// https://stackoverflow.com/a/3437484
#define MAX(a, b)                                                                                                      \
	({                                                                                                                 \
		__typeof__(a) _a = (a);                                                                                        \
		__typeof__(b) _b = (b);                                                                                        \
		_a > _b ? _a : _b;                                                                                             \
	})
#define MIN(a, b)                                                                                                      \
	({                                                                                                                 \
		__typeof__(a) _a = (a);                                                                                        \
		__typeof__(b) _b = (b);                                                                                        \
		_a < _b ? _a : _b;                                                                                             \
	})

void lt_rand_bytes(uint8_t *buf, size_t len);

#ifdef __cplusplus
void hexdump(const uint8_t *buf, size_t len, uint32_t offset = 0, uint8_t width = 16);
#else
void hexdump(const uint8_t *buf, size_t len, uint32_t offset, uint8_t width);
#endif

#ifdef __cplusplus
} // extern "C"
#endif

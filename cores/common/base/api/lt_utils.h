/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#pragma once

#include <libretiny.h>

// https://stackoverflow.com/a/3437484
#define MAX(a, b)               \
	({                          \
		__typeof__(a) _a = (a); \
		__typeof__(b) _b = (b); \
		_a > _b ? _a : _b;      \
	})
#define MIN(a, b)               \
	({                          \
		__typeof__(a) _a = (a); \
		__typeof__(b) _b = (b); \
		_a < _b ? _a : _b;      \
	})

#define LT_MEM32(addr) (*((volatile uint32_t *)(addr)))

// from https://scaryreasoner.wordpress.com/2009/02/28/checking-sizeof-at-compile-time/
// (include/linux/kernel.h)
#define LT_BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2 * !!(condition)]))
#define LT_BUILD_CHECK(condition)  ((void)sizeof(char[1 - 2 * !(condition)]))

/**
 * @brief Generate random bytes using rand().
 *
 * @param buf destination pointer
 * @param len how many bytes to generate
 */
void lt_rand_bytes(uint8_t *buf, size_t len);

/**
 * @brief Print data pointed to by buf in hexdump-like format (hex+ASCII).
 *
 * @param buf source pointer
 * @param len how many bytes to print
 * @param offset increment printed offset by this value
 * @param width how many bytes on a line
 */
void hexdump(
	const uint8_t *buf,
	size_t len,
#ifdef __cplusplus
	uint32_t offset = 0,
	uint8_t width	= 16
#else
	uint32_t offset,
	uint8_t width
#endif
);

/**
 * @brief Convert a byte array to hexadecimal string.
 *
 * @param src source byte array
 * @param len source length (bytes)
 * @param dest destination string
 * @return destination string
 */
char *lt_btox(const uint8_t *src, int len, char *dest);

/**
 * @brief Convert a hexadecimal string to byte array.
 *
 * @param src source string
 * @param len source length (chars)
 * @param dest destination byte array
 * @return destination byte array
 */
uint8_t *lt_xtob(const char *src, int len, uint8_t *dest);

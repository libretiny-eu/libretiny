/* Copyright (c) Kuba Szczodrzyński 2023-03-03. */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <basic_types.h>
#include <platform_conf.h>
#include <utility.h>

int __wrap_memcmp(const void *av, const void *bv, size_t len) {
	return utility_stubs.memcmp(av, bv, len);
}

void *__wrap_memcpy(void *s1, const void *s2, size_t n) {
	return utility_stubs.memcpy(s1, s2, n);
}

void *__wrap_memmove(void *destaddr, const void *sourceaddr, unsigned length) {
	return utility_stubs.memmove(destaddr, sourceaddr, length);
}

void *__wrap_memset(void *dst0, int val, size_t length) {
	return utility_stubs.memset(dst0, val, length);
}

#if (CHIP_VER >= CHIP_B_CUT)
int __wrap_memcmp_s(const void *av, const void *bv, size_t len) {
	return utility_stubs.memcmp_s(av, bv, len);
}
#endif

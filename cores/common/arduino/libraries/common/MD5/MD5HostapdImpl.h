/* Copyright (c) Kuba Szczodrzyński 2022-07-12. */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct MD5Context {
	unsigned long buf[4];
	unsigned long bits[2];
	unsigned char in[64];
};

#define LT_MD5_CTX_T struct MD5Context

#ifdef __cplusplus
} // extern "C"
#endif

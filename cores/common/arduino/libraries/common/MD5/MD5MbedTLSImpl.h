/* Copyright (c) Kuba Szczodrzyński 2022-07-11. */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <mbedtls/md5.h>
#define LT_MD5_CTX_T mbedtls_md5_context

#ifdef __cplusplus
} // extern "C"
#endif

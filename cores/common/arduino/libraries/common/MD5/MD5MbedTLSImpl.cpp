/* Copyright (c) Kuba Szczodrzyński 2022-07-11. */

#if LT_ARD_MD5_MBEDTLS

#include "MD5.h"

extern "C" {

void MD5Init(LT_MD5_CTX_T *context) {
	mbedtls_md5_init(context);
	mbedtls_md5_starts(context);
}

void MD5Update(LT_MD5_CTX_T *context, const unsigned char *buf, unsigned len) {
	mbedtls_md5_update(context, buf, len);
}

void MD5Final(unsigned char digest[16], LT_MD5_CTX_T *context) {
	mbedtls_md5_finish(context, digest);
}

} // extern "C"

#endif // LT_ARD_MD5_MBEDTLS

/* Copyright (c) Kuba Szczodrzyński 2022-07-11. */

#if LT_ARD_MD5_MBEDTLS

extern "C" {

#include <mbedtls/md5.h>

void MD5Init(mbedtls_md5_context *context) {
	mbedtls_md5_init(context);
	mbedtls_md5_starts(context);
}

void MD5Update(mbedtls_md5_context *context, const unsigned char *buf, unsigned len) {
	mbedtls_md5_update(context, buf, len);
}

void MD5Final(unsigned char digest[16], mbedtls_md5_context *context) {
	mbedtls_md5_finish(context, digest);
}

} // extern "C"

#endif // LT_ARD_MD5_MBEDTLS

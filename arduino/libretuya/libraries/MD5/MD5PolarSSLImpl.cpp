/* Copyright (c) Kuba Szczodrzyński 2022-06-03. */

// check if impl defines LT_MD5_USE_POLARSSL
extern "C" {
#include <MD5Impl.h>
}

#if LT_MD5_USE_POLARSSL

#include "MD5PolarSSLImpl.h"

extern "C" {

void MD5Init(LT_MD5_CTX_T *context) {
	md5_init(context);
	md5_starts(context);
}

void MD5Update(LT_MD5_CTX_T *context, const unsigned char *buf, unsigned len) {
	md5_update(context, buf, len);
}

void MD5Final(unsigned char digest[16], LT_MD5_CTX_T *context) {
	md5_finish(context, digest);
}

} // extern "C"

#endif

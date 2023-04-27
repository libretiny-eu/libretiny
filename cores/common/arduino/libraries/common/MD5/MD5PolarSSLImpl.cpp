/* Copyright (c) Kuba Szczodrzyński 2022-06-03. */

#if LT_ARD_MD5_POLARSSL

extern "C" {

#include <polarssl/md5.h>

void MD5Init(md5_context *context) {
	md5_init(context);
	md5_starts(context);
}

void MD5Update(md5_context *context, const unsigned char *buf, unsigned len) {
	md5_update(context, buf, len);
}

void MD5Final(unsigned char digest[16], md5_context *context) {
	md5_finish(context, digest);
}

} // extern "C"

#endif // LT_ARD_MD5_POLARSSL

/* Copyright (c) Kuba Szczodrzyński 2022-06-03. */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	unsigned long total[2];	  /*!< number of bytes processed  */
	unsigned long state[4];	  /*!< intermediate digest state  */
	unsigned char buffer[64]; /*!< data block being processed */
} md5_context;

#define LT_MD5_CTX_T md5_context

#ifdef __cplusplus
} // extern "C"
#endif

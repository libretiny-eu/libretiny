/* Copyright (c) Kuba Szczodrzyński 2022-04-29. */

#include "LibreTuyaAPI.h"

String ipToString(const IPAddress &ip) {
	char szRet[16];
	sprintf(szRet, "%hhu.%hhu.%hhu.%hhu", ip[0], ip[1], ip[2], ip[3]);
	return String(szRet);
}

static void lt_random_bytes(uint8_t *buf, size_t len) {
	int *data = (int *)buf;
	size_t i;
	for (i = 0; len >= sizeof(int); len -= sizeof(int)) {
		data[i++] = rand();
	}
	if (len) {
		int rem				= rand();
		unsigned char *pRem = (unsigned char *)&rem;
		memcpy(buf + i * sizeof(int), pRem, len);
	}
}

/* Copyright (c) Kuba Szczodrzyński 2022-04-29. */

#include "LibreTuyaAPI.h"

__weak char *strdup(const char *s) {
	size_t len = strlen(s) + 1;
	void *newp = malloc(len);
	if (newp == NULL)
		return NULL;
	return (char *)memcpy(newp, s, len);
}

String ipToString(const IPAddress &ip) {
	char szRet[16];
	sprintf(szRet, "%hhu.%hhu.%hhu.%hhu", ip[0], ip[1], ip[2], ip[3]);
	return String(szRet);
}

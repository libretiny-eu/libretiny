/* Copyright (c) Kuba Szczodrzyński 2022-06-04. */

#include "wiring_compat.h"

String ipToString(const IPAddress &ip) {
	char szRet[16];
	sprintf(szRet, "%hhu.%hhu.%hhu.%hhu", ip[0], ip[1], ip[2], ip[3]);
	return String(szRet);
}

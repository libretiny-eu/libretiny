/* Copyright (c) Kuba Szczodrzyński 2022-05-06. */

#include "sdk_mem.h"

#include <Arduino.h> // for memset

void *pvPortZalloc(size_t size) {
	void *pvReturn = pvPortMalloc(size);
	if (pvReturn)
		memset(pvReturn, 0, size);
	return pvReturn;
}

void *pvPortCalloc(size_t nmemb, size_t size) {
	return pvPortZalloc(nmemb * size);
}

/* Copyright (c) Kuba Szczodrzyński 2022-05-06. */

#pragma once

#include <stddef.h>

// provide extern functions directly, as callers
// generally don't expect needing to include malloc()
extern void *pvPortMalloc(size_t xWantedSize);
extern void *pvPortZalloc(size_t size);
extern void *pvPortCalloc(size_t nmemb, size_t size);
extern void *pvPortReAlloc(void *pv, size_t xWantedSize);
extern void vPortFree(void *pv);

#define malloc	pvPortMalloc
#define zalloc	pvPortZalloc
#define calloc	pvPortCalloc
#define realloc pvPortReAlloc
#define free	vPortFree

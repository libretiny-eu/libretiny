/* Copyright (c) Kuba Szczodrzyński 2022-06-18. */

#pragma once

#include <stddef.h>

// Beken SDK is actually pretty good, in terms of declaring
// stdlib functions properly! So no need for any #define hell.
#include <mem_pub.h>

// All the MemMang functions are in stdlib, just wrapped
// during linking.
#include <stdlib.h>
// for memcpy etc.
#include <string.h>

// ...except zalloc, which is apparently not in the stdlib
#define zalloc os_zalloc

#define LT_HEAP_FUNC xPortGetFreeHeapSize

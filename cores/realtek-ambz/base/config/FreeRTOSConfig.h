/* Copyright (c) Kuba Szczodrzyński 2023-03-02. */

#pragma once

#include_next "FreeRTOSConfig.h"

#undef configTOTAL_HEAP_SIZE
#define configTOTAL_HEAP_SIZE ((size_t)(160 * 1024))

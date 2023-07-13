/* Copyright (c) Kuba Szczodrzyński 2023-03-09. */

#pragma once

// This file collects all LibreTiny C API includes.
// The functions are implemented in api/*.c units, which are located
// in the common core, and in the family cores.

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "api/lt_cpu.h"
#include "api/lt_device.h"
#include "api/lt_flash.h"
#include "api/lt_init.h"
#include "api/lt_mem.h"
#include "api/lt_ota.h"
#include "api/lt_sleep.h"
#include "api/lt_utils.h"
#include "api/lt_wdt.h"

#ifdef __cplusplus
} // extern "C"
#endif

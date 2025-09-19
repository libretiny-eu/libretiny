/* Copyright (c) Kuba Szczodrzyński 2023-05-31. */

// - flash_api.c and flash_api_ext.c are both compiled in libambz2_sdk.a
// - the former declares weak functions 'flash_resource_lock' and 'flash_resource_unlock',
//   while the latter actually implements them
// - for some reason, the linker resolves the weak functions only,
//   and doesn't include them in the final binary

#include_next "hal_flash.h"

// try to remove the weak attribute
#undef __weak
#define __weak

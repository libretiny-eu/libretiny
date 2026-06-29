/* Copyright (c) Kuba Szczodrzyński 2023-03-03. */

#pragma once

// Remove SDK definitions of memory utilities.
// Include stdlib versions instead.
// Wrapper functions for the removed prototypes are provided in memory.c.

#include <platform_conf.h>
#include <string.h>
#include <utility.h>

// Map SDK functions so that they point to stdlib again
#define rt_memcmp  memcmp
#define rt_memcpy  memcpy
#define rt_memmove memmove
#define rt_memset  memset

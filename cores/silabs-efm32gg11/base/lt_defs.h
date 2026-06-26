#pragma once

// Silicon Labs EFM32GG11B820F2048GM64 (Cortex-M4F, 72 MHz, 2 MB flash, 512 KB SRAM)
//
// NOTE: lt_defs.h is also consumed by the LibreTiny SCons builder as a config
// file (builder/utils/config.py: LoadDefines). The parser is strict:
//   - Only `#define KEY VALUE` lines are recognized.
//   - VALUE must be a single token (numeric literal or identifier). Multi-token
//     expressions (e.g. `(2048u * 1024u)`) and trailing `//` comments break it.
// Keep all entries here as single-token defines. Anything richer (sizes as
// expressions, derived addresses, etc.) lives in a sibling C/C++ header.

#define LT_HAS_FREERTOS 1
#define LT_HAS_LWIP		1
#define LT_HAS_MBEDTLS	0
// Arduino Update/OTA gate (cores/common Update.cpp). The dual-bank engine
// (lt_ota_* + uf2ota + FAL + bootloader switch/rollback) is implemented and
// bench-proven, so the standard Update.begin/write/end path is enabled.
#define LT_HAS_OTA		1

// __wrap_realloc strategy (common fixups/malloc.c): FreeRTOS heap_4 has no
// realloc, so use the malloc+copy+free fallback. Same choice as beken-72xx.
#define LT_REMALLOC 1

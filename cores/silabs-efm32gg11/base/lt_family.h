#pragma once

#include "lt_defs.h"
#include <lt_pins.h>

// Silicon Labs Gecko SDK pulls in the device header per chip variant.
// This header makes it available across the family's core code.
#include "em_chip.h"
#include "em_cmu.h"
#include "em_device.h"
#include "em_gpio.h"

// Choose the main UART output port. WSTK virtual COM is on USART0 (LOC1) and
// the WGM160P routes the M4 console to the same. Only one UART is exposed in
// Phase 1, so the default port is fixed to 0.
#ifndef LT_UART_DEFAULT_PORT
#if LT_HW_UART0
#define LT_UART_DEFAULT_PORT 0
#else
#error "silabs-efm32gg11: no LT_HW_UART0 — variant must define HAS_SERIAL0"
#endif
#endif

// Auto-download-reboot detection pattern (consumed by Serial.cpp's adrParse
// when LT_AUTO_DOWNLOAD_REBOOT is enabled). FAMILY is defined by the builder
// (frameworks/base.py) as F_<FAMILY_SHORT_NAME>; for silabs-efm32gg11 it
// expands to F_EFM32GG11 via families.json. Pattern matches the convention
// established by beken-72xx, realtek-amb*, lightning-ln882h.
// clang-format off
#define LT_UART_ADR_PATTERN 0x55, 0xAA, \
    (FAMILY >> 24) & 0xFF, \
    (FAMILY >> 16) & 0xFF, \
    (FAMILY >> 8) & 0xFF, \
    (FAMILY >> 0) & 0xFF
// clang-format on

/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#pragma once

#include LT_VARIANT_H

// Choose the main UART output port
#ifndef LT_UART_DEFAULT_PORT
#if HAS_SERIAL2
#define LT_UART_DEFAULT_PORT 2
#elif HAS_SERIAL1
#define LT_UART_DEFAULT_PORT 1
#else
#error "No serial port is available"
#endif
#endif

// Auto-download-reboot detection pattern
// Link check command (CMD_LinkCheck=0)
#define LT_UART_ADR_PATTERN 0x01, 0xE0, 0xFC, 0x01, 0x00

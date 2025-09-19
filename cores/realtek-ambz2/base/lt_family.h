/* Copyright (c) Kuba Szczodrzyński 2023-03-02. */

#pragma once

#include <lt_pins.h>

// Choose the main UART output port
#ifndef LT_UART_DEFAULT_PORT
#if LT_HW_UART2
#define LT_UART_DEFAULT_PORT 2
#elif LT_HW_UART0
#define LT_UART_DEFAULT_PORT 0
#elif LT_HW_UART1
#define LT_UART_DEFAULT_PORT 1
#else
#error "No serial port is available"
#endif
#endif

// Auto-download-reboot detection pattern
// "ping" command for BootROM
#define LT_UART_ADR_PATTERN 'p', 'i', 'n', 'g', '\n'

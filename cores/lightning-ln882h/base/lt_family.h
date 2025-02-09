/* Copyright (c) Etienne Le Cousin 2024-02-10. */

#pragma once

#include <lt_pins.h>

// Choose the main UART output port
#ifndef LT_UART_DEFAULT_PORT
#if LT_HW_UART0
#define LT_UART_DEFAULT_PORT 0
#elif LT_HW_UART1
#define LT_UART_DEFAULT_PORT 1
#elif LT_HW_UART2
#define LT_UART_DEFAULT_PORT 2
#else
#error "No serial port is available"
#endif
#endif

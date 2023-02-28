/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#pragma once

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

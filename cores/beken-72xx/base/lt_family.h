/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#pragma once

// Choose the main UART output port
#ifndef LT_UART_DEFAULT_PORT
#if defined(PIN_SERIAL2_TX)
#define LT_UART_DEFAULT_PORT 2
#else
#define LT_UART_DEFAULT_PORT 1
#endif
#endif

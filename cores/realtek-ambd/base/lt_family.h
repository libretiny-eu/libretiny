/* Copyright (c) Bl00d-B0b 2026-08-06. */

#pragma once

#include <lt_pins.h>

// AmebaD (RTL8720D) KM4 application core. The log console is the LOGUART
// (KM0-owned at boot, handed to KM4) on PA7/PA8, the boot ROM's download pins;
// port 2 in the printf port mapping.
#ifndef LT_UART_DEFAULT_PORT
#define LT_UART_DEFAULT_PORT 2
#endif

// Auto-download-reboot detection needs the AmebaD boot ROM handshake, which the
// AmebaZ "ping" pattern does not match; not captured yet.
// #define LT_UART_ADR_PATTERN

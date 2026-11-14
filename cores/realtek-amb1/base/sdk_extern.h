/* Copyright (c) Kuba Szczodrzyński 2023-05-23. */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int LOGUART_SetBaud(uint32_t BaudRate);	   // from fixups/log_uart.c
void DumpForOneBytes(void *addr, int cnt); // cnt max 0x70!
bool lt_amb1_lwip_ready(void);

#ifdef __cplusplus
} // extern "C"
#endif

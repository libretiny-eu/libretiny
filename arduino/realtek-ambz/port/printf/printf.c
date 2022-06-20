/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <printf_config.h>

#include <printf/printf.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

extern void LOGUART_PutChar(char c);

void putchar_(char c) {
	LOGUART_PutChar(c);
}

WRAP_PRINTF(rtl_printf);
WRAP_PRINTF(DiagPrintf);

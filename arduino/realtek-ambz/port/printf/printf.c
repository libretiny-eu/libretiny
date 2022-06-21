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
WRAP_SPRINTF(rtl_sprintf);
WRAP_SNPRINTF(rtl_snprintf);
WRAP_VSNPRINTF(rtl_vsnprintf);
WRAP_VSNPRINTF(rtl_vsnprintf_r);
WRAP_VPRINTF(rtl_vprintf);
WRAP_PRINTF(DiagPrintf);
WRAP_SPRINTF(DiagSPrintf);
WRAP_SNPRINTF(DiagSnPrintf);
WRAP_PRINTF(prvDiagPrintf);
WRAP_SPRINTF(prvDiagSPrintf);
WRAP_VSPRINTF(VSprintf);
WRAP_PRINTF(LOG_PRINTF);

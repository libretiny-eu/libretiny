/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

// make printf.c define wrapper functions
#define printf_	   __wrap_printf
#define sprintf_   __wrap_sprintf
#define vsprintf_  __wrap_vsprintf
#define snprintf_  __wrap_snprintf
#define vsnprintf_ __wrap_vsnprintf
#define vprintf_   __wrap_vprintf

/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#pragma once

#include <lt_config.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define PRINTF_HAS_DISABLE 1

// make printf.c define wrapper functions
#define printf_	   __wrap_printf
#define sprintf_   __wrap_sprintf
#define vsprintf_  __wrap_vsprintf
#define snprintf_  __wrap_snprintf
#define vsnprintf_ __wrap_vsnprintf
#define vprintf_   __wrap_vprintf

// declare putchar() method with custom output port
void putchar_p(char c, unsigned long port);

#define WRAP_DISABLE_DEF(name)                                                                                         \
	extern void __wrap_##name##_disable();                                                                             \
	extern void __wrap_##name##_enable();                                                                              \
	extern void __wrap_##name##_set(unsigned char disabled);                                                           \
	extern unsigned char __wrap_##name##_get();

#if !LT_UART_SILENT_ENABLED || LT_UART_SILENT_ALL

#define WRAP_DISABLE_DECL(name)                                                                                        \
	void __wrap_##name##_disable() {}                                                                                  \
	void __wrap_##name##_enable() {}                                                                                   \
	void __wrap_##name##_set(unsigned char disabled) {}                                                                \
	unsigned char __wrap_##name##_get() {                                                                              \
		return LT_UART_SILENT_ALL;                                                                                     \
	}

#define WRAP_DISABLE_CHECK(name)                                                                                       \
	{                                                                                                                  \
		if (LT_UART_SILENT_ALL)                                                                                        \
			return 0;                                                                                                  \
	}

#else // LT_UART_SILENT_ENABLED && !LT_UART_SILENT_ALL

#define WRAP_DISABLE_DECL(name)                                                                                        \
	static unsigned char __wrap_##name##_disabled = 0;                                                                 \
	void __wrap_##name##_disable() {                                                                                   \
		__wrap_##name##_disabled = 1;                                                                                  \
	}                                                                                                                  \
	void __wrap_##name##_enable() {                                                                                    \
		__wrap_##name##_disabled = 0;                                                                                  \
	}                                                                                                                  \
	void __wrap_##name##_set(unsigned char disabled) {                                                                 \
		__wrap_##name##_disabled = disabled;                                                                           \
	}                                                                                                                  \
	unsigned char __wrap_##name##_get() {                                                                              \
		return __wrap_##name##_disabled;                                                                               \
	}

#define WRAP_DISABLE_CHECK(name)                                                                                       \
	{                                                                                                                  \
		if (__wrap_##name##_disabled)                                                                                  \
			return 0;                                                                                                  \
	}

#endif // LT_UART_SILENT_ENABLED && !LT_UART_SILENT_ALL

#if LT_UART_SILENT_ALL

#define WRAP_PRINTF(name)                                                                                              \
	WRAP_DISABLE_DECL(name)                                                                                            \
	int __wrap_##name(const char *format, ...) {                                                                       \
		return 0;                                                                                                      \
	}

#define WRAP_VPRINTF(name)                                                                                             \
	WRAP_DISABLE_DECL(name)                                                                                            \
	int __wrap_##name(const char *format, va_list arg) {                                                               \
		return 0;                                                                                                      \
	}

#else // !LT_UART_SILENT_ALL

#define WRAP_PRINTF(name)                                                                                              \
	WRAP_DISABLE_DECL(name)                                                                                            \
	int __wrap_##name(const char *format, ...) {                                                                       \
		WRAP_DISABLE_CHECK(name);                                                                                      \
		va_list va;                                                                                                    \
		va_start(va, format);                                                                                          \
		const int ret = vprintf(format, va);                                                                           \
		va_end(va);                                                                                                    \
		return ret;                                                                                                    \
	}

#define WRAP_VPRINTF(name)                                                                                             \
	WRAP_DISABLE_DECL(name)                                                                                            \
	int __wrap_##name(const char *format, va_list arg) {                                                               \
		WRAP_DISABLE_CHECK(name);                                                                                      \
		return vprintf(format, arg);                                                                                   \
	}

#endif // !LT_UART_SILENT_ALL

#define WRAP_SPRINTF(name)                                                                                             \
	int __wrap_##name(char *s, const char *format, ...) {                                                              \
		va_list va;                                                                                                    \
		va_start(va, format);                                                                                          \
		const int ret = vsprintf(s, format, va);                                                                       \
		va_end(va);                                                                                                    \
		return ret;                                                                                                    \
	}

#define WRAP_SNPRINTF(name)                                                                                            \
	int __wrap_##name(char *s, size_t count, const char *format, ...) {                                                \
		va_list va;                                                                                                    \
		va_start(va, format);                                                                                          \
		const int ret = vsnprintf(s, count, format, va);                                                               \
		va_end(va);                                                                                                    \
		return ret;                                                                                                    \
	}

#define WRAP_VSPRINTF(name)                                                                                            \
	int __wrap_##name(char *s, const char *format, va_list arg) {                                                      \
		return vsprintf(s, format, arg);                                                                               \
	}

#define WRAP_VSNPRINTF(name)                                                                                           \
	int __wrap_##name(char *s, size_t count, const char *format, va_list arg) {                                        \
		return vsnprintf(s, count, format, arg);                                                                       \
	}

#ifdef __cplusplus
} // extern "C"
#endif

/* Copyright (c) Kuba Szczodrzyński 2022-04-28. */

#include "lt_logger.h"

#include <stdarg.h>
#include <stdint.h>

#if LT_HAS_PRINTF
#include <printf/printf.h>
#include <printf_port.h>
#else
#include <stdio.h>
#endif

#if (LT_LOGGER_TIMESTAMP || LT_LOGGER_TASK) && LT_HAS_FREERTOS
#include <FreeRTOS.h>
#include <task.h>
#endif

#define COLOR_FMT			 "\e[0;30m"
#define COLOR_BLACK			 0x00
#define COLOR_RED			 0x01
#define COLOR_GREEN			 0x02
#define COLOR_YELLOW		 0x03
#define COLOR_BLUE			 0x04
#define COLOR_MAGENTA		 0x05
#define COLOR_CYAN			 0x06
#define COLOR_WHITE			 0x07
#define COLOR_BRIGHT_BLACK	 0x10
#define COLOR_BRIGHT_RED	 0x11
#define COLOR_BRIGHT_GREEN	 0x12
#define COLOR_BRIGHT_YELLOW	 0x13
#define COLOR_BRIGHT_BLUE	 0x14
#define COLOR_BRIGHT_MAGENTA 0x15
#define COLOR_BRIGHT_CYAN	 0x16
#define COLOR_BRIGHT_WHITE	 0x17

static uint32_t uart_port  = LT_UART_DEFAULT_LOGGER;
static const char levels[] = {'V', 'D', 'I', 'W', 'E', 'F'};

#if LT_LOGGER_COLOR
static const uint8_t colors[] = {
	COLOR_BRIGHT_CYAN,
	COLOR_BRIGHT_BLUE,
	COLOR_BRIGHT_GREEN,
	COLOR_BRIGHT_YELLOW,
	COLOR_BRIGHT_RED,
	COLOR_BRIGHT_MAGENTA,
};
#endif

#if LIBRETUYA_ARDUINO
unsigned long millis(void);
#endif

#if LT_LOGGER_CALLER
void lt_log(const uint8_t level, const char *caller, const unsigned short line, const char *format, ...) {
#else
void lt_log(const uint8_t level, const char *format, ...) {
#endif

	if (uart_port == 0xFF)
		return;

#if LT_LOGGER_TIMESTAMP
#if LIBRETUYA_ARDUINO
	float seconds = millis() / 1000.0f;
#elif LT_HAS_FREERTOS
	float seconds = xTaskGetTickCount() * portTICK_PERIOD_MS / 1000.0f;
#else
	float seconds = 0;
#endif
#if LT_PRINTF_BROKEN
	char zero[4] = "\x00\x30\x30";
	if (seconds == 0.0f)
		zero[0] = '0';
#endif
#endif

#if LT_LOGGER_TASK && LT_HAS_FREERTOS
	char task_colon	  = ':';
	TaskHandle_t task = xTaskGetCurrentTaskHandle();
	char *task_name	  = pcTaskGetTaskName(task);
	if (!task) {
		task_name  = "";
		task_colon = '-';
	}
#endif

#if LT_LOGGER_COLOR
	char c_bright = '0' + (colors[level] >> 4);
	char c_value  = '0' + (colors[level] & 0x7);
#endif

#if LT_HAS_PRINTF
	fctprintf(
		(void (*)(char, void *))putchar_p,
		(void *)uart_port,
#else
	printf(
#endif
	// format:
#if LT_LOGGER_COLOR
		"\e[%c;3%cm"
#endif
		"%c "
#if LT_LOGGER_TIMESTAMP
#if LT_PRINTF_BROKEN
		"[%11.3f%s] "
#else
		"[%11.3f] "
#endif
#endif
#if LT_LOGGER_COLOR
		"\e[0m"
#endif
#if LT_LOGGER_CALLER
		"%s():%hu: "
#endif
#if LT_LOGGER_TASK && LT_HAS_FREERTOS
		"%s%c "
#endif
		,
	// arguments:
#if LT_LOGGER_COLOR
		c_bright, // whether text is bright
		c_value,  // text color
#endif
		levels[level]
#if LT_LOGGER_TIMESTAMP
		,
		seconds // float
#if LT_PRINTF_BROKEN
		,
		zero // append missing zeroes if printf "%11.3f" prints "0."
#endif
#endif
#if LT_LOGGER_CALLER
		,
		caller,
		line
#endif
#if LT_LOGGER_TASK && LT_HAS_FREERTOS
		,
		task_name,
		task_colon // printing outside of tasks
#endif
	);

#if LT_HAS_PRINTF
	va_list va_args;
	va_start(va_args, format);
	vfctprintf((void (*)(char, void *))putchar_p, (void *)uart_port, format, va_args);
	va_end(va_args);
	putchar_p('\r', uart_port);
	putchar_p('\n', uart_port);
#else
	va_list va_args;
	va_start(va_args, format);
	vprintf(format, va_args);
	va_end(va_args);
	putchar('\r');
	putchar('\n');
#endif
}

void lt_log_set_port(uint8_t port) {
	uart_port = port;
}

void lt_log_disable() {
	uart_port = 0xFF;
}

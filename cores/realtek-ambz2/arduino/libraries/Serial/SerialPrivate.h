/* Copyright (c) Kuba Szczodrzyński 2023-05-24. */

#pragma once

#include <Arduino.h>
#include <sdk_private.h>

typedef struct {
	hal_uart_adapter_t *uart;
	RingBuffer buf;
} SerialData;

#define DATA   ((SerialData *)data)
#define pDATA  ((SerialData *)param)
#define BUF	   (DATA->buf)
#define pdBUF  (pDATA->buf)
#define UART   (DATA->uart)
#define pdUART (pDATA->uart)

/* Copyright (c) Kuba Szczodrzyński 2023-09-23. */

#pragma once

#include <ArduinoPrivate.h>
#include <SPI.h>

struct SPIData {
	SSI_InitTypeDef init;
	SPI_TypeDef *spi;
	IRQn irq;
};

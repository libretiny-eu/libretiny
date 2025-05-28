/* Copyright (c) Kuba Szczodrzyński 2023-03-12. */

// Fix for PeripheralNames.h producing errors on GCC newer than 5.x.x.
// The struct pointer casts are replaced with register addresses, which fixes compilation.
// On older versions, this change doesn't make any difference.
// MBED_PERIPHERALNAMES_H is defined in the SDK builder, to eliminate PeripheralNames.h completely.

#include_next "cmsis.h"

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	UART_0 = (int)UART0_REG_BASE,
	UART_1 = (int)UART1_REG_BASE,
	UART_2 = (int)UART2_REG_BASE,
} UARTName;

typedef enum {
	ADC0_0 = 0,
	ADC0_1,
	ADC0_2,
	ADC0_3
} ADCName;

typedef enum {
	SPI_0 = (int)SPI0_REG_BASE,
	SPI_1 = (int)SPI1_REG_BASE,
} SPIName;

typedef enum {
	I2C_0 = (int)I2C0_REG_BASE,
	I2C_1 = (int)I2C1_REG_BASE,
} I2CName;

typedef enum {
	PWM_0 = 1,
	PWM_1,
	PWM_2,
	PWM_3,
	PWM_4,
	PWM_5
} PWMName;

#ifdef __cplusplus
}
#endif

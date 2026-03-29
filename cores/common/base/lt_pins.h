/* Copyright (c) Kuba Szczodrzyński 2023-06-21. */

#include LT_VARIANT_H

#define PIN_INVALID 255

#define LT_HW_UART0 HAS_SERIAL0
#define LT_HW_UART1 HAS_SERIAL1
#define LT_HW_UART2 HAS_SERIAL2
#define LT_HW_I2C0	HAS_WIRE0
#define LT_HW_I2C1	HAS_WIRE1
#define LT_HW_I2C2	HAS_WIRE2
#define LT_HW_SPI0	HAS_SPI0
#define LT_HW_SPI1	HAS_SPI1
#define LT_HW_SPI2	HAS_SPI2

#if LT_HW_UART0
#ifndef PIN_SERIAL0_RX
#define PIN_SERIAL0_RX PIN_INVALID
#endif
#ifndef PIN_SERIAL0_TX
#define PIN_SERIAL0_TX PIN_INVALID
#endif
#endif

#if LT_HW_UART1
#ifndef PIN_SERIAL1_RX
#define PIN_SERIAL1_RX PIN_INVALID
#endif
#ifndef PIN_SERIAL1_TX
#define PIN_SERIAL1_TX PIN_INVALID
#endif
#endif

#if LT_HW_UART2
#ifndef PIN_SERIAL2_RX
#define PIN_SERIAL2_RX PIN_INVALID
#endif
#ifndef PIN_SERIAL2_TX
#define PIN_SERIAL2_TX PIN_INVALID
#endif
#endif

#if LT_HW_I2C0
#ifndef PIN_WIRE0_SDA
#define PIN_WIRE0_SDA PIN_INVALID
#endif
#ifndef PIN_WIRE0_SCL
#define PIN_WIRE0_SCL PIN_INVALID
#endif
#endif

#if LT_HW_I2C1
#ifndef PIN_WIRE1_SDA
#define PIN_WIRE1_SDA PIN_INVALID
#endif
#ifndef PIN_WIRE1_SCL
#define PIN_WIRE1_SCL PIN_INVALID
#endif
#endif

#if LT_HW_I2C2
#ifndef PIN_WIRE2_SDA
#define PIN_WIRE2_SDA PIN_INVALID
#endif
#ifndef PIN_WIRE2_SCL
#define PIN_WIRE2_SCL PIN_INVALID
#endif
#endif

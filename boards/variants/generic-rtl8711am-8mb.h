/* Minimal Ameba1 bench variant: SDK pin IDs, not module header positions. */
#pragma once

#define PINS_COUNT		   7
#define NUM_DIGITAL_PINS   7
#define NUM_ANALOG_INPUTS  0
#define NUM_ANALOG_OUTPUTS 0
#define PINS_GPIO_MAX	   35

#define PIN_SERIAL0_RX	6u
#define PIN_SERIAL0_TX	7u
#define PINS_SERIAL0_RX {6u}
#define PINS_SERIAL0_TX {7u}

/* Logical Serial2 is the dedicated log UART, not SDK high-speed UART2. */
#define PIN_SERIAL2_RX	17u
#define PIN_SERIAL2_TX	16u
#define PINS_SERIAL2_RX {17u}
#define PINS_SERIAL2_TX {16u}

#define HAS_SERIAL0				1
#define HAS_SERIAL1				0
#define HAS_SERIAL2				1
#define HAS_SPI0				0
#define HAS_SPI1				0
#define HAS_SPI2				0
#define HAS_WIRE0				0
#define HAS_WIRE1				0
#define HAS_WIRE2				0
#define SERIAL_INTERFACES_COUNT 2
#define SPI_INTERFACES_COUNT	0
#define WIRE_INTERFACES_COUNT	0

#define PIN_PA06 6u
#define PIN_PA07 7u
#define PIN_PB00 16u
#define PIN_PB01 17u

/* Factory status indicator: PC_2 = SDK pin id 0x22 (34), active-HIGH. */
#define PIN_PC02 34u
#define PIN_PC03 35u
#define PIN_PC00 32u

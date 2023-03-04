/* This file was auto-generated from generic-rtl8720cf-2mb-992k.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         20
#define NUM_DIGITAL_PINS   20
#define NUM_ANALOG_OUTPUTS 0

// SPI Interfaces
// --------------
#define SPI_INTERFACES_COUNT 1
#define PIN_SPI0_CS_0        2u  // PIN_A2
#define PIN_SPI0_CS_1        5u  // PIN_A7
#define PIN_SPI0_CS_2        13u // PIN_A15
#define PIN_SPI0_MISO_0      8u  // PIN_A10
#define PIN_SPI0_MISO_1      18u // PIN_A20
#define PIN_SPI0_MOSI_0      4u  // PIN_A4
#define PIN_SPI0_MOSI_1      7u  // PIN_A9
#define PIN_SPI0_MOSI_2      17u // PIN_A19
#define PIN_SPI0_SCK_0       3u  // PIN_A3
#define PIN_SPI0_SCK_1       6u  // PIN_A8
#define PIN_SPI0_SCK_2       14u // PIN_A16

// Wire Interfaces
// ---------------
#define WIRE_INTERFACES_COUNT 1
#define PIN_WIRE0_SCL_0       2u  // PIN_A2
#define PIN_WIRE0_SCL_1       9u  // PIN_A11
#define PIN_WIRE0_SCL_2       13u // PIN_A15
#define PIN_WIRE0_SCL_3       17u // PIN_A19
#define PIN_WIRE0_SDA_0       3u  // PIN_A3
#define PIN_WIRE0_SDA_1       10u // PIN_A12
#define PIN_WIRE0_SDA_2       14u // PIN_A16
#define PIN_WIRE0_SDA_3       18u // PIN_A20

// Serial ports
// ------------
#define SERIAL_INTERFACES_COUNT 3
#define PIN_SERIAL0_CTS         8u  // PIN_A10
#define PIN_SERIAL0_RTS         7u  // PIN_A9
#define PIN_SERIAL0_RX_0        10u // PIN_A12
#define PIN_SERIAL0_RX_1        11u // PIN_A13
#define PIN_SERIAL0_TX_0        9u  // PIN_A11
#define PIN_SERIAL0_TX_1        12u // PIN_A14
#define PIN_SERIAL1_CTS         4u  // PIN_A4
#define PIN_SERIAL1_RX_0        0u  // PIN_A0
#define PIN_SERIAL1_RX_1        2u  // PIN_A2
#define PIN_SERIAL1_TX_0        1u  // PIN_A1
#define PIN_SERIAL1_TX_1        3u  // PIN_A3
#define PIN_SERIAL2_CTS         17u // PIN_A19
#define PIN_SERIAL2_RTS         18u // PIN_A20
#define PIN_SERIAL2_RX          13u // PIN_A15
#define PIN_SERIAL2_TX          14u // PIN_A16

// Pin function macros
// -------------------
#define PIN_CTS0 8u  // PIN_A10
#define PIN_CTS1 4u  // PIN_A4
#define PIN_CTS2 17u // PIN_A19
#define PIN_PA00 0u  // PIN_A0
#define PIN_PA01 1u  // PIN_A1
#define PIN_PA02 2u  // PIN_A2
#define PIN_PA03 3u  // PIN_A3
#define PIN_PA04 4u  // PIN_A4
#define PIN_PA07 5u  // PIN_A7
#define PIN_PA08 6u  // PIN_A8
#define PIN_PA09 7u  // PIN_A9
#define PIN_PA10 8u  // PIN_A10
#define PIN_PA11 9u  // PIN_A11
#define PIN_PA12 10u // PIN_A12
#define PIN_PA13 11u // PIN_A13
#define PIN_PA14 12u // PIN_A14
#define PIN_PA15 13u // PIN_A15
#define PIN_PA16 14u // PIN_A16
#define PIN_PA17 15u // PIN_A17
#define PIN_PA18 16u // PIN_A18
#define PIN_PA19 17u // PIN_A19
#define PIN_PA20 18u // PIN_A20
#define PIN_PA23 19u // PIN_A23
#define PIN_RTS0 7u  // PIN_A9
#define PIN_RTS2 18u // PIN_A20

// Port availability
// -----------------
#define HAS_SERIAL0 1
#define HAS_SERIAL1 1
#define HAS_SERIAL2 1
#define HAS_SPI0    1
#define HAS_WIRE0   1

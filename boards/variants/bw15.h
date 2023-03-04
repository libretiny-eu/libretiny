/* This file was auto-generated from bw15.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         13
#define NUM_DIGITAL_PINS   13
#define NUM_ANALOG_OUTPUTS 0

// SPI Interfaces
// --------------
#define SPI_INTERFACES_COUNT 1
#define PIN_SPI0_CS_0        2u // PIN_A2
#define PIN_SPI0_CS_1        3u // PIN_A15
#define PIN_SPI0_MISO        6u // PIN_A20
#define PIN_SPI0_MOSI_0      4u // PIN_A4
#define PIN_SPI0_MOSI_1      5u // PIN_A19
#define PIN_SPI0_SCK_0       7u // PIN_A16
#define PIN_SPI0_SCK_1       9u // PIN_A3

// Wire Interfaces
// ---------------
#define WIRE_INTERFACES_COUNT 1
#define PIN_WIRE0_SCL_0       2u // PIN_A2
#define PIN_WIRE0_SCL_1       3u // PIN_A15
#define PIN_WIRE0_SCL_2       5u // PIN_A19
#define PIN_WIRE0_SDA_0       6u // PIN_A20
#define PIN_WIRE0_SDA_1       7u // PIN_A16
#define PIN_WIRE0_SDA_2       9u // PIN_A3

// Serial ports
// ------------
#define SERIAL_INTERFACES_COUNT 3
#define PIN_SERIAL0_RX          11u // PIN_A13
#define PIN_SERIAL0_TX          12u // PIN_A14
#define PIN_SERIAL1_CTS         4u  // PIN_A4
#define PIN_SERIAL1_RX_0        2u  // PIN_A2
#define PIN_SERIAL1_RX_1        8u  // PIN_A0
#define PIN_SERIAL1_TX_0        9u  // PIN_A3
#define PIN_SERIAL1_TX_1        10u // PIN_A1
#define PIN_SERIAL2_CTS         5u  // PIN_A19
#define PIN_SERIAL2_RTS         6u  // PIN_A20
#define PIN_SERIAL2_RX          3u  // PIN_A15
#define PIN_SERIAL2_TX          7u  // PIN_A16

// Pin function macros
// -------------------
#define PIN_CTS1  4u  // PIN_A4
#define PIN_CTS2  5u  // PIN_A19
#define PIN_MISO0 6u  // PIN_A20
#define PIN_PA00  8u  // PIN_A0
#define PIN_PA01  10u // PIN_A1
#define PIN_PA02  2u  // PIN_A2
#define PIN_PA03  9u  // PIN_A3
#define PIN_PA04  4u  // PIN_A4
#define PIN_PA13  11u // PIN_A13
#define PIN_PA14  12u // PIN_A14
#define PIN_PA15  3u  // PIN_A15
#define PIN_PA16  7u  // PIN_A16
#define PIN_PA17  0u  // PIN_A17
#define PIN_PA18  1u  // PIN_A18
#define PIN_PA19  5u  // PIN_A19
#define PIN_PA20  6u  // PIN_A20
#define PIN_RTS2  6u  // PIN_A20

// Port availability
// -----------------
#define HAS_SERIAL0 1
#define HAS_SERIAL1 1
#define HAS_SERIAL2 1
#define HAS_SPI0    1
#define HAS_WIRE0   1

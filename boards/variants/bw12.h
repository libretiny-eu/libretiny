/* This file was auto-generated from bw12.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         11
#define NUM_DIGITAL_PINS   11
#define NUM_ANALOG_INPUTS  1
#define NUM_ANALOG_OUTPUTS 0

// Analog pins
// -----------
#define PIN_A0 3u     // PA_19
#define A0     PIN_A0

// SPI Interfaces
// --------------
#define SPI_INTERFACES_COUNT 2
#define PIN_SPI0_CS          3u  // PA_19
#define PIN_SPI0_MISO        4u  // PA_22
#define PIN_SPI0_MOSI        10u // PA_23
#define PIN_SPI0_SCK         9u  // PA_18
#define PIN_SPI1_CS          3u  // PA_19
#define PIN_SPI1_MISO        4u  // PA_22
#define PIN_SPI1_MOSI        10u // PA_23
#define PIN_SPI1_SCK         9u  // PA_18

// Wire Interfaces
// ---------------
#define WIRE_INTERFACES_COUNT 2
#define PIN_WIRE0_SCL_0       1u  // PA_29
#define PIN_WIRE0_SCL_1       4u  // PA_22
#define PIN_WIRE0_SDA_0       3u  // PA_19
#define PIN_WIRE0_SDA_1       5u  // PA_30
#define PIN_WIRE1_SCL         9u  // PA_18
#define PIN_WIRE1_SDA         10u // PA_23

// Serial ports
// ------------
#define SERIAL_INTERFACES_COUNT 2
#define PIN_SERIAL0_CTS         3u  // PA_19
#define PIN_SERIAL0_RTS         4u  // PA_22
#define PIN_SERIAL0_RX          9u  // PA_18
#define PIN_SERIAL0_TX          10u // PA_23
#define PIN_SERIAL2_RX          1u  // PA_29
#define PIN_SERIAL2_TX          5u  // PA_30

// Pin function macros
// -------------------
#define PIN_ADC1  3u  // PA_19
#define PIN_CS0   3u  // PA_19
#define PIN_CS1   3u  // PA_19
#define PIN_CTS0  3u  // PA_19
#define PIN_MISO0 4u  // PA_22
#define PIN_MISO1 4u  // PA_22
#define PIN_MOSI0 10u // PA_23
#define PIN_MOSI1 10u // PA_23
#define PIN_PA00  2u  // PA_0
#define PIN_PA05  0u  // PA_5
#define PIN_PA12  7u  // PA_12
#define PIN_PA14  6u  // PA_14
#define PIN_PA15  8u  // PA_15
#define PIN_PA18  9u  // PA_18
#define PIN_PA19  3u  // PA_19
#define PIN_PA22  4u  // PA_22
#define PIN_PA23  10u // PA_23
#define PIN_PA29  1u  // PA_29
#define PIN_PA30  5u  // PA_30
#define PIN_RTS0  4u  // PA_22
#define PIN_RX0   9u  // PA_18
#define PIN_RX2   1u  // PA_29
#define PIN_SCK0  9u  // PA_18
#define PIN_SCK1  9u  // PA_18
#define PIN_TX0   10u // PA_23
#define PIN_TX2   5u  // PA_30

// Port availability
// -----------------
#define HAS_SERIAL0 1
#define HAS_SERIAL2 1
#define HAS_SPI0    1
#define HAS_SPI1    1
#define HAS_WIRE0   1
#define HAS_WIRE1   1

/* This file was auto-generated from wr3l.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         12
#define NUM_DIGITAL_PINS   11
#define NUM_ANALOG_INPUTS  2
#define NUM_ANALOG_OUTPUTS 0

// Analog pins
// -----------
#define PIN_A0 1u     // PA_19
#define PIN_A1 11u    // AD_2
#define A0     PIN_A0
#define A1     PIN_A1

// SPI Interfaces
// --------------
#define SPI_INTERFACES_COUNT 2
#define PIN_SPI0_CS          1u  // PA_19
#define PIN_SPI0_MISO        0u  // PA_22
#define PIN_SPI0_MOSI        10u // PA_23
#define PIN_SPI0_SCK         9u  // PA_18
#define PIN_SPI1_CS          1u  // PA_19
#define PIN_SPI1_MISO        0u  // PA_22
#define PIN_SPI1_MOSI        10u // PA_23
#define PIN_SPI1_SCK         9u  // PA_18

// Wire Interfaces
// ---------------
#define WIRE_INTERFACES_COUNT 2
#define PIN_WIRE0_SCL_0       0u  // PA_22
#define PIN_WIRE0_SCL_1       5u  // PA_29
#define PIN_WIRE0_SDA_0       1u  // PA_19
#define PIN_WIRE0_SDA_1       6u  // PA_30
#define PIN_WIRE1_SCL         9u  // PA_18
#define PIN_WIRE1_SDA         10u // PA_23

// Serial ports
// ------------
#define SERIAL_INTERFACES_COUNT 2
#define PIN_SERIAL0_CTS         1u  // PA_19
#define PIN_SERIAL0_RTS         0u  // PA_22
#define PIN_SERIAL0_RX          9u  // PA_18
#define PIN_SERIAL0_TX          10u // PA_23
#define PIN_SERIAL2_RX          5u  // PA_29
#define PIN_SERIAL2_TX          6u  // PA_30

// Pin function macros
// -------------------
#define PIN_ADC1  1u  // PA_19
#define PIN_ADC2  11u // AD_2
#define PIN_CS0   1u  // PA_19
#define PIN_CS1   1u  // PA_19
#define PIN_CTS0  1u  // PA_19
#define PIN_MISO0 0u  // PA_22
#define PIN_MISO1 0u  // PA_22
#define PIN_MOSI0 10u // PA_23
#define PIN_MOSI1 10u // PA_23
#define PIN_PA00  4u  // PA_0
#define PIN_PA05  7u  // PA_5
#define PIN_PA12  8u  // PA_12
#define PIN_PA14  2u  // PA_14
#define PIN_PA15  3u  // PA_15
#define PIN_PA18  9u  // PA_18
#define PIN_PA19  1u  // PA_19
#define PIN_PA22  0u  // PA_22
#define PIN_PA23  10u // PA_23
#define PIN_PA29  5u  // PA_29
#define PIN_PA30  6u  // PA_30
#define PIN_RTS0  0u  // PA_22
#define PIN_RX0   9u  // PA_18
#define PIN_RX2   5u  // PA_29
#define PIN_SCK0  9u  // PA_18
#define PIN_SCK1  9u  // PA_18
#define PIN_TX0   10u // PA_23
#define PIN_TX2   6u  // PA_30

// Port availability
// -----------------
#define HAS_SERIAL0 1
#define HAS_SERIAL2 1
#define HAS_SPI0    1
#define HAS_SPI1    1
#define HAS_WIRE0   1
#define HAS_WIRE1   1

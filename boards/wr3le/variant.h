/* This file was auto-generated from wr3le.json using boardgen */

#pragma once

#include <WVariant.h>

// clang-format off

// Pins
// ----
#define PINS_COUNT         12
#define NUM_ANALOG_INPUTS  2
#define NUM_DIGITAL_PINS   11
#define NUM_ANALOG_OUTPUTS 0

// Analog pins
// -----------
#define PIN_A1 11u    // AD_2
#define PIN_A0 6u     // PA_19
#define A1     PIN_A1
#define A0     PIN_A0

// SPI Interfaces
// --------------
#define SPI_INTERFACES_COUNT 2
#define PIN_SPI0_CS          6u  // PA_19
#define PIN_SPI0_MISO        3u  // PA_22
#define PIN_SPI0_MOSI        10u // PA_23
#define PIN_SPI0_SCK         9u  // PA_18
#define PIN_SPI1_CS          6u  // PA_19
#define PIN_SPI1_MISO        3u  // PA_22
#define PIN_SPI1_MOSI        10u // PA_23
#define PIN_SPI1_SCK         9u  // PA_18

// Wire Interfaces
// ---------------
#define WIRE_INTERFACES_COUNT 2
#define PIN_WIRE0_SCL_0       0u  // PA_29
#define PIN_WIRE0_SCL_1       3u  // PA_22
#define PIN_WIRE0_SDA_0       5u  // PA_30
#define PIN_WIRE0_SDA_1       6u  // PA_19
#define PIN_WIRE1_SCL         9u  // PA_18
#define PIN_WIRE1_SDA         10u // PA_23

// Serial ports
// ------------
#define SERIAL_INTERFACES_COUNT 2
#define PIN_SERIAL0_CTS         6u  // PA_19
#define PIN_SERIAL0_RTS         3u  // PA_22
#define PIN_SERIAL0_RX          9u  // PA_18
#define PIN_SERIAL0_TX          10u // PA_23
#define PIN_SERIAL2_RX          0u  // PA_29
#define PIN_SERIAL2_TX          5u  // PA_30

// Pin function macros
// -------------------
#define PIN_FUNCTION_ADC1  6u  // PA_19
#define PIN_FUNCTION_ADC2  11u // AD_2
#define PIN_FUNCTION_CS0   6u  // PA_19
#define PIN_FUNCTION_CS1   6u  // PA_19
#define PIN_FUNCTION_CTS0  6u  // PA_19
#define PIN_FUNCTION_MISO0 3u  // PA_22
#define PIN_FUNCTION_MISO1 3u  // PA_22
#define PIN_FUNCTION_MOSI0 10u // PA_23
#define PIN_FUNCTION_MOSI1 10u // PA_23
#define PIN_FUNCTION_PA00  4u  // PA_0
#define PIN_FUNCTION_PA05  7u  // PA_5
#define PIN_FUNCTION_PA12  8u  // PA_12
#define PIN_FUNCTION_PA14  1u  // PA_14
#define PIN_FUNCTION_PA15  2u  // PA_15
#define PIN_FUNCTION_PA18  9u  // PA_18
#define PIN_FUNCTION_PA19  6u  // PA_19
#define PIN_FUNCTION_PA22  3u  // PA_22
#define PIN_FUNCTION_PA23  10u // PA_23
#define PIN_FUNCTION_PA29  0u  // PA_29
#define PIN_FUNCTION_PA30  5u  // PA_30
#define PIN_FUNCTION_RTS0  3u  // PA_22
#define PIN_FUNCTION_RX0   9u  // PA_18
#define PIN_FUNCTION_RX2   0u  // PA_29
#define PIN_FUNCTION_SCK0  9u  // PA_18
#define PIN_FUNCTION_SCK1  9u  // PA_18
#define PIN_FUNCTION_TX0   10u // PA_23
#define PIN_FUNCTION_TX2   5u  // PA_30

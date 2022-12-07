/* This file was auto-generated from generic-rtl8710bn-2mb-788k.json using boardgen */

#pragma once

#include <WVariant.h>

// clang-format off

// Pins
// ----
#define PINS_COUNT         18
#define NUM_DIGITAL_PINS   17
#define NUM_ANALOG_INPUTS  2
#define NUM_ANALOG_OUTPUTS 0

// Analog pins
// -----------
#define PIN_A0 12u    // PA_19
#define PIN_A1 17u    // AD_2
#define A0     PIN_A0
#define A1     PIN_A1

// SPI Interfaces
// --------------
#define SPI_INTERFACES_COUNT 2
#define PIN_SPI0_CS          12u // PA_19
#define PIN_SPI0_MISO        13u // PA_22
#define PIN_SPI0_MOSI        14u // PA_23
#define PIN_SPI0_SCK         11u // PA_18
#define PIN_SPI1_CS          12u // PA_19
#define PIN_SPI1_MISO        13u // PA_22
#define PIN_SPI1_MOSI        14u // PA_23
#define PIN_SPI1_SCK         11u // PA_18

// Wire Interfaces
// ---------------
#define WIRE_INTERFACES_COUNT 2
#define PIN_WIRE0_SCL_0       13u // PA_22
#define PIN_WIRE0_SCL_1       15u // PA_29
#define PIN_WIRE0_SDA_0       12u // PA_19
#define PIN_WIRE0_SDA_1       16u // PA_30
#define PIN_WIRE1_SCL         11u // PA_18
#define PIN_WIRE1_SDA         14u // PA_23

// Serial ports
// ------------
#define SERIAL_INTERFACES_COUNT 2
#define PIN_SERIAL0_CTS         12u // PA_19
#define PIN_SERIAL0_RTS         13u // PA_22
#define PIN_SERIAL0_RX          11u // PA_18
#define PIN_SERIAL0_TX          14u // PA_23
#define PIN_SERIAL2_RX          15u // PA_29
#define PIN_SERIAL2_TX          16u // PA_30

// Pin function macros
// -------------------
#define PIN_FUNCTION_ADC1  12u // PA_19
#define PIN_FUNCTION_ADC2  17u // AD_2
#define PIN_FUNCTION_CS0   12u // PA_19
#define PIN_FUNCTION_CS1   12u // PA_19
#define PIN_FUNCTION_CTS0  12u // PA_19
#define PIN_FUNCTION_FCS   2u  // PA_6
#define PIN_FUNCTION_FD0   5u  // PA_9
#define PIN_FUNCTION_FD1   3u  // PA_7
#define PIN_FUNCTION_FD2   4u  // PA_8
#define PIN_FUNCTION_FD3   7u  // PA_11
#define PIN_FUNCTION_FSCK  6u  // PA_10
#define PIN_FUNCTION_MISO0 13u // PA_22
#define PIN_FUNCTION_MISO1 13u // PA_22
#define PIN_FUNCTION_MOSI0 14u // PA_23
#define PIN_FUNCTION_MOSI1 14u // PA_23
#define PIN_FUNCTION_PA00  0u  // PA_0
#define PIN_FUNCTION_PA05  1u  // PA_5
#define PIN_FUNCTION_PA06  2u  // PA_6
#define PIN_FUNCTION_PA07  3u  // PA_7
#define PIN_FUNCTION_PA08  4u  // PA_8
#define PIN_FUNCTION_PA09  5u  // PA_9
#define PIN_FUNCTION_PA10  6u  // PA_10
#define PIN_FUNCTION_PA11  7u  // PA_11
#define PIN_FUNCTION_PA12  8u  // PA_12
#define PIN_FUNCTION_PA14  9u  // PA_14
#define PIN_FUNCTION_PA15  10u // PA_15
#define PIN_FUNCTION_PA18  11u // PA_18
#define PIN_FUNCTION_PA19  12u // PA_19
#define PIN_FUNCTION_PA22  13u // PA_22
#define PIN_FUNCTION_PA23  14u // PA_23
#define PIN_FUNCTION_PA29  15u // PA_29
#define PIN_FUNCTION_PA30  16u // PA_30
#define PIN_FUNCTION_RTS0  13u // PA_22
#define PIN_FUNCTION_RX0   11u // PA_18
#define PIN_FUNCTION_RX2   15u // PA_29
#define PIN_FUNCTION_SCK0  11u // PA_18
#define PIN_FUNCTION_SCK1  11u // PA_18
#define PIN_FUNCTION_TX0   14u // PA_23
#define PIN_FUNCTION_TX2   16u // PA_30

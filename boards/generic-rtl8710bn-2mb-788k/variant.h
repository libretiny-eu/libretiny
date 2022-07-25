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

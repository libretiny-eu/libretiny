/* This file was auto-generated from wr3n.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         10
#define NUM_ANALOG_INPUTS  1
#define NUM_DIGITAL_PINS   9
#define NUM_ANALOG_OUTPUTS 0

// Analog pins
// -----------
#define PIN_A1 9u     // AD_2
#define A1     PIN_A1

// SPI Interfaces
// --------------
#define SPI_INTERFACES_COUNT 0

// Wire Interfaces
// ---------------
#define WIRE_INTERFACES_COUNT 2
#define PIN_WIRE0_SCL         0u // PA_29
#define PIN_WIRE0_SDA         4u // PA_30
#define PIN_WIRE1_SCL         7u // PA_18
#define PIN_WIRE1_SDA         8u // PA_23

// Serial ports
// ------------
#define SERIAL_INTERFACES_COUNT 2
#define PIN_SERIAL0_RX          7u // PA_18
#define PIN_SERIAL0_TX          8u // PA_23
#define PIN_SERIAL2_RX          0u // PA_29
#define PIN_SERIAL2_TX          4u // PA_30

// Pin function macros
// -------------------
#define PIN_ADC2  9u // AD_2
#define PIN_MOSI0 8u // PA_23
#define PIN_MOSI1 8u // PA_23
#define PIN_PA00  3u // PA_0
#define PIN_PA05  5u // PA_5
#define PIN_PA12  6u // PA_12
#define PIN_PA14  1u // PA_14
#define PIN_PA15  2u // PA_15
#define PIN_PA18  7u // PA_18
#define PIN_PA23  8u // PA_23
#define PIN_PA29  0u // PA_29
#define PIN_PA30  4u // PA_30
#define PIN_RX0   7u // PA_18
#define PIN_RX2   0u // PA_29
#define PIN_SCK0  7u // PA_18
#define PIN_SCK1  7u // PA_18
#define PIN_SCL0  0u // PA_29
#define PIN_SCL1  7u // PA_18
#define PIN_SDA0  4u // PA_30
#define PIN_SDA1  8u // PA_23
#define PIN_TX0   8u // PA_23
#define PIN_TX2   4u // PA_30

// Port availability
// -----------------
#define HAS_SERIAL0 1
#define HAS_SERIAL2 1
#define HAS_WIRE0   1
#define HAS_WIRE1   1

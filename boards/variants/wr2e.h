/* This file was auto-generated from wr2e.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         8
#define NUM_DIGITAL_PINS   7
#define NUM_ANALOG_INPUTS  2
#define NUM_ANALOG_OUTPUTS 0

// Analog pins
// -----------
#define PIN_A0 1u     // PA_19
#define PIN_A1 7u     // AD_2
#define A0     PIN_A0
#define A1     PIN_A1

// SPI Interfaces
// --------------
#define SPI_INTERFACES_COUNT 0

// Wire Interfaces
// ---------------
#define WIRE_INTERFACES_COUNT 1
#define PIN_WIRE1_SCL         3u // PA_18
#define PIN_WIRE1_SDA         4u // PA_23

// Serial ports
// ------------
#define SERIAL_INTERFACES_COUNT 1
#define PIN_SERIAL0_CTS         1u // PA_19
#define PIN_SERIAL0_RX          3u // PA_18
#define PIN_SERIAL0_TX          4u // PA_23

// Pin function macros
// -------------------
#define PIN_ADC1  1u // PA_19
#define PIN_ADC2  7u // AD_2
#define PIN_CS0   1u // PA_19
#define PIN_CS1   1u // PA_19
#define PIN_CTS0  1u // PA_19
#define PIN_MOSI0 4u // PA_23
#define PIN_MOSI1 4u // PA_23
#define PIN_PA05  2u // PA_5
#define PIN_PA12  0u // PA_12
#define PIN_PA14  5u // PA_14
#define PIN_PA15  6u // PA_15
#define PIN_PA18  3u // PA_18
#define PIN_PA19  1u // PA_19
#define PIN_PA23  4u // PA_23
#define PIN_RX0   3u // PA_18
#define PIN_SCK0  3u // PA_18
#define PIN_SCK1  3u // PA_18
#define PIN_SCL1  3u // PA_18
#define PIN_SDA0  1u // PA_19
#define PIN_SDA1  4u // PA_23
#define PIN_TX0   4u // PA_23

// Port availability
// -----------------
#define HAS_SERIAL0 1
#define HAS_WIRE1   1

/* This file was auto-generated from cb3se.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         17
#define NUM_DIGITAL_PINS   17
#define NUM_ANALOG_INPUTS  1
#define NUM_ANALOG_OUTPUTS 0

// Analog pins
// -----------
#define PIN_A0 0u     // GPIO23
#define A0     PIN_A0

// SPI Interfaces
// --------------
#define SPI_INTERFACES_COUNT 0

// Wire Interfaces
// ---------------
#define WIRE_INTERFACES_COUNT 1
#define PIN_WIRE2_SCL         6u // GPIO0
#define PIN_WIRE2_SDA         7u // GPIO1

// Serial ports
// ------------
#define SERIAL_INTERFACES_COUNT 2
#define PIN_SERIAL1_RX          10u // GPIO10
#define PIN_SERIAL1_TX          11u // GPIO11
#define PIN_SERIAL2_RX          7u  // GPIO1
#define PIN_SERIAL2_TX          6u  // GPIO0

// Pin function macros
// -------------------
#define PIN_ADC3 0u  // GPIO23
#define PIN_CS   12u // GPIO15
#define PIN_MISO 15u // GPIO17
#define PIN_MOSI 16u // GPIO16
#define PIN_P0   6u  // GPIO0
#define PIN_P1   7u  // GPIO1
#define PIN_P6   4u  // GPIO6
#define PIN_P7   9u  // GPIO7
#define PIN_P8   8u  // GPIO8
#define PIN_P9   5u  // GPIO9
#define PIN_P10  10u // GPIO10
#define PIN_P11  11u // GPIO11
#define PIN_P14  1u  // GPIO14
#define PIN_P15  12u // GPIO15
#define PIN_P16  16u // GPIO16
#define PIN_P17  15u // GPIO17
#define PIN_P20  14u // GPIO20
#define PIN_P22  13u // GPIO22
#define PIN_P23  0u  // GPIO23
#define PIN_P24  3u  // GPIO24
#define PIN_P26  2u  // GPIO26
#define PIN_PWM0 4u  // GPIO6
#define PIN_PWM1 9u  // GPIO7
#define PIN_PWM2 8u  // GPIO8
#define PIN_PWM3 5u  // GPIO9
#define PIN_PWM4 3u  // GPIO24
#define PIN_PWM5 2u  // GPIO26
#define PIN_RX1  10u // GPIO10
#define PIN_RX2  7u  // GPIO1
#define PIN_SCK  1u  // GPIO14
#define PIN_SCL1 14u // GPIO20
#define PIN_SCL2 6u  // GPIO0
#define PIN_SDA2 7u  // GPIO1
#define PIN_TX1  11u // GPIO11
#define PIN_TX2  6u  // GPIO0

// Port availability
// -----------------
#define HAS_SERIAL1 1
#define HAS_SERIAL2 1
#define HAS_WIRE2   1

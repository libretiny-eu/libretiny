/* This file was auto-generated from lsc-lma35.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         15
#define NUM_DIGITAL_PINS   15
#define NUM_ANALOG_INPUTS  1
#define NUM_ANALOG_OUTPUTS 0

// Analog pins
// -----------
#define PIN_A0 6u     // GPIO23
#define A0     PIN_A0

// SPI Interfaces
// --------------
#define SPI_INTERFACES_COUNT 0

// Wire Interfaces
// ---------------
#define WIRE_INTERFACES_COUNT 1
#define PIN_WIRE2_SCL         5u  // GPIO0
#define PIN_WIRE2_SDA         14u // GPIO1

// Serial ports
// ------------
#define SERIAL_INTERFACES_COUNT 2
#define PIN_SERIAL1_RX          12u // GPIO10
#define PIN_SERIAL1_TX          13u // GPIO11
#define PIN_SERIAL2_RX          14u // GPIO1
#define PIN_SERIAL2_TX          5u  // GPIO0

// Pin function macros
// -------------------
#define PIN_ADC3 6u  // GPIO23
#define PIN_MOSI 2u  // GPIO16
#define PIN_P0   5u  // GPIO0
#define PIN_P1   14u // GPIO1
#define PIN_P6   10u // GPIO6
#define PIN_P7   11u // GPIO7
#define PIN_P8   7u  // GPIO8
#define PIN_P9   8u  // GPIO9
#define PIN_P10  12u // GPIO10
#define PIN_P11  13u // GPIO11
#define PIN_P14  1u  // GPIO14
#define PIN_P16  2u  // GPIO16
#define PIN_P21  9u  // GPIO21
#define PIN_P22  4u  // GPIO22
#define PIN_P23  6u  // GPIO23
#define PIN_P24  3u  // GPIO24
#define PIN_P26  0u  // GPIO26
#define PIN_PWM0 10u // GPIO6
#define PIN_PWM1 11u // GPIO7
#define PIN_PWM2 7u  // GPIO8
#define PIN_PWM3 8u  // GPIO9
#define PIN_PWM4 3u  // GPIO24
#define PIN_PWM5 0u  // GPIO26
#define PIN_RX1  12u // GPIO10
#define PIN_RX2  14u // GPIO1
#define PIN_SCK  1u  // GPIO14
#define PIN_SCL2 5u  // GPIO0
#define PIN_SDA1 9u  // GPIO21
#define PIN_SDA2 14u // GPIO1
#define PIN_TX1  13u // GPIO11
#define PIN_TX2  5u  // GPIO0

// Port availability
// -----------------
#define HAS_SERIAL1 1
#define HAS_SERIAL2 1
#define HAS_WIRE2   1

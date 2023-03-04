/* This file was auto-generated from wb2s.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         14
#define NUM_DIGITAL_PINS   14
#define NUM_ANALOG_INPUTS  1
#define NUM_ANALOG_OUTPUTS 0

// Analog pins
// -----------
#define PIN_A0 3u     // GPIO23
#define PIN_A0 3u     // GPIO23
#define A0     PIN_A0
#define A0     PIN_A0

// SPI Interfaces
// --------------
#define SPI_INTERFACES_COUNT 0

// Wire Interfaces
// ---------------
#define WIRE_INTERFACES_COUNT 2
#define PIN_WIRE1_SCL         8u  // GPIO20
#define PIN_WIRE1_SDA         12u // GPIO21
#define PIN_WIRE2_SCL         11u // GPIO0
#define PIN_WIRE2_SDA         10u // GPIO1

// Serial ports
// ------------
#define SERIAL_INTERFACES_COUNT 2
#define PIN_SERIAL1_RX          4u  // GPIO10
#define PIN_SERIAL1_TX          5u  // GPIO11
#define PIN_SERIAL2_RX          10u // GPIO1
#define PIN_SERIAL2_TX          11u // GPIO0

// Pin function macros
// -------------------
#define PIN_ADC3 3u  // GPIO23
#define PIN_P0   11u // GPIO0
#define PIN_P1   10u // GPIO1
#define PIN_P6   2u  // GPIO6
#define PIN_P7   1u  // GPIO7
#define PIN_P8   0u  // GPIO8
#define PIN_P9   9u  // GPIO9
#define PIN_P10  4u  // GPIO10
#define PIN_P11  5u  // GPIO11
#define PIN_P20  8u  // GPIO20
#define PIN_P21  12u // GPIO21
#define PIN_P22  13u // GPIO22
#define PIN_P23  3u  // GPIO23
#define PIN_P24  6u  // GPIO24
#define PIN_P26  7u  // GPIO26
#define PIN_PWM0 2u  // GPIO6
#define PIN_PWM1 1u  // GPIO7
#define PIN_PWM2 0u  // GPIO8
#define PIN_PWM3 9u  // GPIO9
#define PIN_PWM4 6u  // GPIO24
#define PIN_PWM5 7u  // GPIO26
#define PIN_RX1  4u  // GPIO10
#define PIN_RX2  10u // GPIO1
#define PIN_SCL1 8u  // GPIO20
#define PIN_SCL2 11u // GPIO0
#define PIN_SDA1 12u // GPIO21
#define PIN_SDA2 10u // GPIO1
#define PIN_TX1  5u  // GPIO11
#define PIN_TX2  11u // GPIO0

// Port availability
// -----------------
#define HAS_SERIAL1 1
#define HAS_SERIAL2 1
#define HAS_WIRE1   1
#define HAS_WIRE2   1

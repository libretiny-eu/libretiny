/* This file was auto-generated from wb2l.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         13
#define NUM_DIGITAL_PINS   13
#define NUM_ANALOG_INPUTS  1
#define NUM_ANALOG_OUTPUTS 0

// Analog pins
// -----------
#define PIN_A0 11u    // GPIO23
#define A0     PIN_A0

// SPI Interfaces
// --------------
#define SPI_INTERFACES_COUNT 0

// Wire Interfaces
// ---------------
#define WIRE_INTERFACES_COUNT 2
#define PIN_WIRE1_SCL         9u  // GPIO20
#define PIN_WIRE1_SDA         10u // GPIO21
#define PIN_WIRE2_SCL         8u  // GPIO0
#define PIN_WIRE2_SDA         7u  // GPIO1

// Serial ports
// ------------
#define SERIAL_INTERFACES_COUNT 2
#define PIN_SERIAL1_RX          5u // GPIO10
#define PIN_SERIAL1_TX          6u // GPIO11
#define PIN_SERIAL2_RX          7u // GPIO1
#define PIN_SERIAL2_TX          8u // GPIO0

// Pin function macros
// -------------------
#define PIN_ADC3 11u // GPIO23
#define PIN_P0   8u  // GPIO0
#define PIN_P1   7u  // GPIO1
#define PIN_P6   2u  // GPIO6
#define PIN_P7   1u  // GPIO7
#define PIN_P8   0u  // GPIO8
#define PIN_P10  5u  // GPIO10
#define PIN_P11  6u  // GPIO11
#define PIN_P20  9u  // GPIO20
#define PIN_P21  10u // GPIO21
#define PIN_P22  12u // GPIO22
#define PIN_P23  11u // GPIO23
#define PIN_P24  4u  // GPIO24
#define PIN_P26  3u  // GPIO26
#define PIN_PWM0 2u  // GPIO6
#define PIN_PWM1 1u  // GPIO7
#define PIN_PWM2 0u  // GPIO8
#define PIN_PWM4 4u  // GPIO24
#define PIN_PWM5 3u  // GPIO26
#define PIN_RX1  5u  // GPIO10
#define PIN_RX2  7u  // GPIO1
#define PIN_SCL1 9u  // GPIO20
#define PIN_SCL2 8u  // GPIO0
#define PIN_SDA1 10u // GPIO21
#define PIN_SDA2 7u  // GPIO1
#define PIN_TX1  6u  // GPIO11
#define PIN_TX2  8u  // GPIO0

// Port availability
// -----------------
#define HAS_SERIAL1 1
#define HAS_SERIAL2 1
#define HAS_WIRE1   1
#define HAS_WIRE2   1

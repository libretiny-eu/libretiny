/* This file was auto-generated from cb3se.json using boardgen */

#pragma once

#include <WVariant.h>

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
#define PIN_FUNCTION_ADC3 0u  // GPIO23
#define PIN_FUNCTION_CS   12u // GPIO15
#define PIN_FUNCTION_MISO 15u // GPIO17
#define PIN_FUNCTION_MOSI 16u // GPIO16
#define PIN_FUNCTION_P0   6u  // GPIO0
#define PIN_FUNCTION_P1   7u  // GPIO1
#define PIN_FUNCTION_P6   4u  // GPIO6
#define PIN_FUNCTION_P7   9u  // GPIO7
#define PIN_FUNCTION_P8   8u  // GPIO8
#define PIN_FUNCTION_P9   5u  // GPIO9
#define PIN_FUNCTION_P10  10u // GPIO10
#define PIN_FUNCTION_P11  11u // GPIO11
#define PIN_FUNCTION_P14  1u  // GPIO14
#define PIN_FUNCTION_P15  12u // GPIO15
#define PIN_FUNCTION_P16  16u // GPIO16
#define PIN_FUNCTION_P17  15u // GPIO17
#define PIN_FUNCTION_P20  14u // GPIO20
#define PIN_FUNCTION_P22  13u // GPIO22
#define PIN_FUNCTION_P23  0u  // GPIO23
#define PIN_FUNCTION_P24  3u  // GPIO24
#define PIN_FUNCTION_P26  2u  // GPIO26
#define PIN_FUNCTION_PWM0 4u  // GPIO6
#define PIN_FUNCTION_PWM1 9u  // GPIO7
#define PIN_FUNCTION_PWM2 8u  // GPIO8
#define PIN_FUNCTION_PWM3 5u  // GPIO9
#define PIN_FUNCTION_PWM4 3u  // GPIO24
#define PIN_FUNCTION_PWM5 2u  // GPIO26
#define PIN_FUNCTION_RX1  10u // GPIO10
#define PIN_FUNCTION_RX2  7u  // GPIO1
#define PIN_FUNCTION_SCK  1u  // GPIO14
#define PIN_FUNCTION_SCL1 14u // GPIO20
#define PIN_FUNCTION_SCL2 6u  // GPIO0
#define PIN_FUNCTION_SDA2 7u  // GPIO1
#define PIN_FUNCTION_TX1  11u // GPIO11
#define PIN_FUNCTION_TX2  6u  // GPIO0

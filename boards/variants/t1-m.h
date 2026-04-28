/* This file was auto-generated from t1-m.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         9  // Total GPIO count
#define NUM_DIGITAL_PINS   9  // Digital inputs/outputs
#define NUM_ANALOG_INPUTS  4  // ADC inputs
#define NUM_ANALOG_OUTPUTS 5  // PWM & DAC outputs
#define PINS_GPIO_MAX      26 // Last usable GPIO number

// Wire Interfaces
// ---------------
#define PIN_WIRE2_SCL  24u // GPIO24
#define PIN_WIRE2_SDA  26u // GPIO26
#define PINS_WIRE2_SCL {24u}
#define PINS_WIRE2_SDA {26u}

// Serial ports
// ------------
#define PIN_SERIAL1_RX  10u // GPIO10
#define PIN_SERIAL1_TX  11u // GPIO11
#define PIN_SERIAL2_RX  1u  // GPIO1
#define PIN_SERIAL2_TX  0u  // GPIO0
#define PINS_SERIAL1_RX {10u}
#define PINS_SERIAL1_TX {11u}
#define PINS_SERIAL2_RX {1u}
#define PINS_SERIAL2_TX {0u}

// Pin function macros
// -------------------
#define PIN_ADC1 26u // GPIO26
#define PIN_ADC2 24u // GPIO24
#define PIN_ADC5 1u  // GPIO1
#define PIN_ADC6 10u // GPIO10
#define PIN_P0   0u  // GPIO0
#define PIN_P1   1u  // GPIO1
#define PIN_P6   6u  // GPIO6
#define PIN_P8   8u  // GPIO8
#define PIN_P9   9u  // GPIO9
#define PIN_P10  10u // GPIO10
#define PIN_P11  11u // GPIO11
#define PIN_P24  24u // GPIO24
#define PIN_P26  26u // GPIO26
#define PIN_PWM0 6u  // GPIO6
#define PIN_PWM2 8u  // GPIO8
#define PIN_PWM3 9u  // GPIO9
#define PIN_PWM4 24u // GPIO24
#define PIN_PWM5 26u // GPIO26
#define PIN_RX1  10u // GPIO10
#define PIN_RX2  1u  // GPIO1
#define PIN_SCL2 24u // GPIO24
#define PIN_SDA2 26u // GPIO26
#define PIN_TX1  11u // GPIO11
#define PIN_TX2  0u  // GPIO0

// Port availability
// -----------------
#define HAS_SERIAL1             1
#define HAS_SERIAL2             1
#define HAS_WIRE2               1
#define SERIAL_INTERFACES_COUNT 2
#define WIRE_INTERFACES_COUNT   1

// Arduino pin names
// -----------------
#define PIN_D0  26u // GPIO26
#define PIN_D1  6u  // GPIO6
#define PIN_D2  8u  // GPIO8
#define PIN_D3  1u  // GPIO1
#define PIN_D4  10u // GPIO10
#define PIN_D5  11u // GPIO11
#define PIN_D6  9u  // GPIO9
#define PIN_D7  24u // GPIO24
#define PIN_D11 0u  // GPIO0
#define PIN_A0  26u // GPIO26
#define PIN_A1  10u // GPIO10
#define PIN_A2  1u  // GPIO1
#define PIN_A3  24u // GPIO24

// Static pin names
// ----------------
static const unsigned char A0 = PIN_A0;
static const unsigned char A1 = PIN_A1;
static const unsigned char A2 = PIN_A2;
static const unsigned char A3 = PIN_A3;
static const unsigned char D0 = PIN_D0;
static const unsigned char D1 = PIN_D1;
static const unsigned char D2 = PIN_D2;
static const unsigned char D3 = PIN_D3;
static const unsigned char D4 = PIN_D4;
static const unsigned char D5 = PIN_D5;
static const unsigned char D6 = PIN_D6;
static const unsigned char D7 = PIN_D7;
static const unsigned char D11 = PIN_D11;

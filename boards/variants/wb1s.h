/* This file was auto-generated from wb1s.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         11 // Total GPIO count
#define NUM_DIGITAL_PINS   11 // Digital inputs/outputs
#define NUM_ANALOG_INPUTS  1  // ADC inputs
#define NUM_ANALOG_OUTPUTS 6  // PWM & DAC outputs
#define PINS_GPIO_MAX      26 // Last usable GPIO number

// Wire Interfaces
// ---------------
#define PIN_WIRE2_SCL  0u // GPIO0
#define PIN_WIRE2_SDA  1u // GPIO1
#define PINS_WIRE2_SCL (pin_size_t[]){0u}
#define PINS_WIRE2_SDA (pin_size_t[]){1u}

// Serial ports
// ------------
#define PIN_SERIAL1_RX  10u // GPIO10
#define PIN_SERIAL1_TX  11u // GPIO11
#define PIN_SERIAL2_RX  1u  // GPIO1
#define PIN_SERIAL2_TX  0u  // GPIO0
#define PINS_SERIAL1_RX (pin_size_t[]){10u}
#define PINS_SERIAL1_TX (pin_size_t[]){11u}
#define PINS_SERIAL2_RX (pin_size_t[]){1u}
#define PINS_SERIAL2_TX (pin_size_t[]){0u}

// Pin function macros
// -------------------
#define PIN_ADC3 23u // GPIO23
#define PIN_P0   0u  // GPIO0
#define PIN_P1   1u  // GPIO1
#define PIN_P6   6u  // GPIO6
#define PIN_P7   7u  // GPIO7
#define PIN_P8   8u  // GPIO8
#define PIN_P9   9u  // GPIO9
#define PIN_P10  10u // GPIO10
#define PIN_P11  11u // GPIO11
#define PIN_P23  23u // GPIO23
#define PIN_P24  24u // GPIO24
#define PIN_P26  26u // GPIO26
#define PIN_PWM0 6u  // GPIO6
#define PIN_PWM1 7u  // GPIO7
#define PIN_PWM2 8u  // GPIO8
#define PIN_PWM3 9u  // GPIO9
#define PIN_PWM4 24u // GPIO24
#define PIN_PWM5 26u // GPIO26
#define PIN_RX1  10u // GPIO10
#define PIN_RX2  1u  // GPIO1
#define PIN_SCL2 0u  // GPIO0
#define PIN_SDA2 1u  // GPIO1
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
#define PIN_D0  11u // GPIO11
#define PIN_D1  10u // GPIO10
#define PIN_D2  26u // GPIO26
#define PIN_D3  24u // GPIO24
#define PIN_D4  0u  // GPIO0
#define PIN_D5  8u  // GPIO8
#define PIN_D6  7u  // GPIO7
#define PIN_D7  1u  // GPIO1
#define PIN_D8  9u  // GPIO9
#define PIN_D9  6u  // GPIO6
#define PIN_D10 23u // GPIO23
#define PIN_A0  23u // GPIO23

// Static pin names
// ----------------
static const unsigned char A0 = PIN_A0;
static const unsigned char D0 = PIN_D0;
static const unsigned char D1 = PIN_D1;
static const unsigned char D2 = PIN_D2;
static const unsigned char D3 = PIN_D3;
static const unsigned char D4 = PIN_D4;
static const unsigned char D5 = PIN_D5;
static const unsigned char D6 = PIN_D6;
static const unsigned char D7 = PIN_D7;
static const unsigned char D8 = PIN_D8;
static const unsigned char D9 = PIN_D9;
static const unsigned char D10 = PIN_D10;

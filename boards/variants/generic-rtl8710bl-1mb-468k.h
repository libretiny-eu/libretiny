/* This file was auto-generated from generic-rtl8710bl-1mb-468k.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         10 // Total GPIO count
#define NUM_DIGITAL_PINS   10 // Digital inputs/outputs
#define NUM_ANALOG_INPUTS  0  // ADC inputs
#define NUM_ANALOG_OUTPUTS 5  // PWM & DAC outputs
#define PINS_GPIO_MAX      30 // Last usable GPIO number

// Wire Interfaces
// ---------------
#define PIN_WIRE0_SCL  29u // PA_29
#define PIN_WIRE0_SDA  30u // PA_30
#define PIN_WIRE1_SCL  18u // PA_18
#define PIN_WIRE1_SDA  23u // PA_23
#define PINS_WIRE0_SCL {29u}
#define PINS_WIRE0_SDA {30u}
#define PINS_WIRE1_SCL {18u}
#define PINS_WIRE1_SDA {23u}

// Serial ports
// ------------
#define PIN_SERIAL0_RX  18u // PA_18
#define PIN_SERIAL0_TX  23u // PA_23
#define PIN_SERIAL2_RX  29u // PA_29
#define PIN_SERIAL2_TX  30u // PA_30
#define PINS_SERIAL0_RX {18u}
#define PINS_SERIAL0_TX {23u}
#define PINS_SERIAL2_RX {29u}
#define PINS_SERIAL2_TX {30u}

// Pin function macros
// -------------------
#define PIN_FCS  6u  // PA_6
#define PIN_FD0  9u  // PA_9
#define PIN_FD1  7u  // PA_7
#define PIN_FSCK 10u // PA_10
#define PIN_PA06 6u  // PA_6
#define PIN_PA07 7u  // PA_7
#define PIN_PA09 9u  // PA_9
#define PIN_PA10 10u // PA_10
#define PIN_PA14 14u // PA_14
#define PIN_PA15 15u // PA_15
#define PIN_PA18 18u // PA_18
#define PIN_PA23 23u // PA_23
#define PIN_PA29 29u // PA_29
#define PIN_PA30 30u // PA_30
#define PIN_PWM1 15u // PA_15
#define PIN_PWM3 30u // PA_30
#define PIN_PWM4 29u // PA_29
#define PIN_RX0  18u // PA_18
#define PIN_RX2  29u // PA_29
#define PIN_SCL0 29u // PA_29
#define PIN_SCL1 18u // PA_18
#define PIN_SDA0 30u // PA_30
#define PIN_SDA1 23u // PA_23
#define PIN_TX0  23u // PA_23
#define PIN_TX2  30u // PA_30

// Port availability
// -----------------
#define HAS_SERIAL0             1
#define HAS_SERIAL2             1
#define HAS_WIRE0               1
#define HAS_WIRE1               1
#define SERIAL_INTERFACES_COUNT 2
#define WIRE_INTERFACES_COUNT   2

// Arduino pin names
// -----------------
#define PIN_D2  6u  // PA_6
#define PIN_D3  7u  // PA_7
#define PIN_D5  9u  // PA_9
#define PIN_D6  10u // PA_10
#define PIN_D9  14u // PA_14
#define PIN_D10 15u // PA_15
#define PIN_D11 18u // PA_18
#define PIN_D14 23u // PA_23
#define PIN_D15 29u // PA_29
#define PIN_D16 30u // PA_30

// Static pin names
// ----------------
static const unsigned char D2 = PIN_D2;
static const unsigned char D3 = PIN_D3;
static const unsigned char D5 = PIN_D5;
static const unsigned char D6 = PIN_D6;
static const unsigned char D9 = PIN_D9;
static const unsigned char D10 = PIN_D10;
static const unsigned char D11 = PIN_D11;
static const unsigned char D14 = PIN_D14;
static const unsigned char D15 = PIN_D15;
static const unsigned char D16 = PIN_D16;

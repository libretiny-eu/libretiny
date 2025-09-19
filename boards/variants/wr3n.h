/* This file was auto-generated from wr3n.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         10 // Total GPIO count
#define NUM_DIGITAL_PINS   9  // Digital inputs/outputs
#define NUM_ANALOG_INPUTS  1  // ADC inputs
#define NUM_ANALOG_OUTPUTS 8  // PWM & DAC outputs
#define PINS_GPIO_MAX      41 // Last usable GPIO number

// Wire Interfaces
// ---------------
#define PIN_WIRE0_SCL  29u // PA_29
#define PIN_WIRE0_SDA  30u // PA_30
#define PIN_WIRE1_SCL  18u // PA_18
#define PIN_WIRE1_SDA  23u // PA_23
#define PINS_WIRE0_SCL (pin_size_t[]){29u}
#define PINS_WIRE0_SDA (pin_size_t[]){30u}
#define PINS_WIRE1_SCL (pin_size_t[]){18u}
#define PINS_WIRE1_SDA (pin_size_t[]){23u}

// Serial ports
// ------------
#define PIN_SERIAL0_RX  18u // PA_18
#define PIN_SERIAL0_TX  23u // PA_23
#define PIN_SERIAL2_RX  29u // PA_29
#define PIN_SERIAL2_TX  30u // PA_30
#define PINS_SERIAL0_RX (pin_size_t[]){18u}
#define PINS_SERIAL0_TX (pin_size_t[]){23u}
#define PINS_SERIAL2_RX (pin_size_t[]){29u}
#define PINS_SERIAL2_TX (pin_size_t[]){30u}

// Pin function macros
// -------------------
#define PIN_ADC2  41u // AD_2
#define PIN_MOSI0 23u // PA_23
#define PIN_MOSI1 23u // PA_23
#define PIN_PA00  0u  // PA_0
#define PIN_PA05  5u  // PA_5
#define PIN_PA12  12u // PA_12
#define PIN_PA14  14u // PA_14
#define PIN_PA15  15u // PA_15
#define PIN_PA18  18u // PA_18
#define PIN_PA23  23u // PA_23
#define PIN_PA29  29u // PA_29
#define PIN_PA30  30u // PA_30
#define PIN_PWM1  15u // PA_15
#define PIN_PWM2  0u  // PA_0
#define PIN_PWM3  12u // PA_12
#define PIN_PWM4  5u  // PA_5
#define PIN_RX0   18u // PA_18
#define PIN_RX2   29u // PA_29
#define PIN_SCK0  18u // PA_18
#define PIN_SCK1  18u // PA_18
#define PIN_SCL0  29u // PA_29
#define PIN_SCL1  18u // PA_18
#define PIN_SDA0  30u // PA_30
#define PIN_SDA1  23u // PA_23
#define PIN_TX0   23u // PA_23
#define PIN_TX2   30u // PA_30

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
#define PIN_D0 29u // PA_29
#define PIN_D1 14u // PA_14
#define PIN_D2 15u // PA_15
#define PIN_D3 0u  // PA_0
#define PIN_D4 30u // PA_30
#define PIN_D5 5u  // PA_5
#define PIN_D6 12u // PA_12
#define PIN_D7 18u // PA_18
#define PIN_D8 23u // PA_23
#define PIN_A1 41u // AD_2

// Static pin names
// ----------------
static const unsigned char A1 = PIN_A1;
static const unsigned char D0 = PIN_D0;
static const unsigned char D1 = PIN_D1;
static const unsigned char D2 = PIN_D2;
static const unsigned char D3 = PIN_D3;
static const unsigned char D4 = PIN_D4;
static const unsigned char D5 = PIN_D5;
static const unsigned char D6 = PIN_D6;
static const unsigned char D7 = PIN_D7;
static const unsigned char D8 = PIN_D8;

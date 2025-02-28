/* This file was auto-generated from wl2s.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         13 // Total GPIO count
#define NUM_DIGITAL_PINS   13 // Digital inputs/outputs
#define NUM_ANALOG_INPUTS  3  // ADC inputs
#define NUM_ANALOG_OUTPUTS 0  // PWM & DAC outputs
#define PINS_GPIO_MAX      25 // Last usable GPIO number

// Wire Interfaces
// ---------------
#define PIN_WIRE0_SCL_0  7u  // PA_7
#define PIN_WIRE0_SCL_1  12u // PA_12
#define PIN_WIRE0_SCL_2  3u  // PA_3
#define PIN_WIRE0_SCL_3  10u // PA_10
#define PIN_WIRE0_SCL_4  2u  // PA_2
#define PIN_WIRE0_SCL_5  0u  // PA_0
#define PIN_WIRE0_SCL_6  19u // PB_3
#define PIN_WIRE0_SCL_7  11u // PA_11
#define PIN_WIRE0_SCL_8  9u  // PA_9
#define PIN_WIRE0_SCL_9  24u // PB_8
#define PIN_WIRE0_SCL_10 25u // PB_9
#define PIN_WIRE0_SCL_11 5u  // PA_5
#define PIN_WIRE0_SCL_12 1u  // PA_1
#define PIN_WIRE0_SDA_0  7u  // PA_7
#define PIN_WIRE0_SDA_1  12u // PA_12
#define PIN_WIRE0_SDA_2  3u  // PA_3
#define PIN_WIRE0_SDA_3  10u // PA_10
#define PIN_WIRE0_SDA_4  2u  // PA_2
#define PIN_WIRE0_SDA_5  0u  // PA_0
#define PIN_WIRE0_SDA_6  19u // PB_3
#define PIN_WIRE0_SDA_7  11u // PA_11
#define PIN_WIRE0_SDA_8  9u  // PA_9
#define PIN_WIRE0_SDA_9  24u // PB_8
#define PIN_WIRE0_SDA_10 25u // PB_9
#define PIN_WIRE0_SDA_11 5u  // PA_5
#define PIN_WIRE0_SDA_12 1u  // PA_1
#define PINS_WIRE0_SCL   (pin_size_t[]){7u, 12u, 3u, 10u, 2u, 0u, 19u, 11u, 9u, 24u, 25u, 5u, 1u}
#define PINS_WIRE0_SDA   (pin_size_t[]){7u, 12u, 3u, 10u, 2u, 0u, 19u, 11u, 9u, 24u, 25u, 5u, 1u}

// Serial ports
// ------------
#define PIN_SERIAL0_RX  3u  // PA_3
#define PIN_SERIAL0_TX  2u  // PA_2
#define PIN_SERIAL1_RX  24u // PB_8
#define PIN_SERIAL1_TX  25u // PB_9
#define PINS_SERIAL0_RX (pin_size_t[]){3u}
#define PINS_SERIAL0_TX (pin_size_t[]){2u}
#define PINS_SERIAL1_RX (pin_size_t[]){24u}
#define PINS_SERIAL1_TX (pin_size_t[]){25u}

// Pin function macros
// -------------------
#define PIN_ADC2 0u  // PA_0
#define PIN_ADC3 1u  // PA_1
#define PIN_ADC5 19u // PB_3
#define PIN_PA00 0u  // PA_0
#define PIN_PA01 1u  // PA_1
#define PIN_PA02 2u  // PA_2
#define PIN_PA03 3u  // PA_3
#define PIN_PA05 5u  // PA_5
#define PIN_PA07 7u  // PA_7
#define PIN_PA09 9u  // PA_9
#define PIN_PA10 10u // PA_10
#define PIN_PA11 11u // PA_11
#define PIN_PA12 12u // PA_12
#define PIN_PB03 19u // PB_3
#define PIN_PB08 24u // PB_8
#define PIN_PB09 25u // PB_9
#define PIN_RX0  3u  // PA_3
#define PIN_RX1  24u // PB_8
#define PIN_SCL0 1u  // PA_1
#define PIN_SDA0 1u  // PA_1
#define PIN_TX0  2u  // PA_2
#define PIN_TX1  25u // PB_9

// Port availability
// -----------------
#define HAS_SERIAL0             1
#define HAS_SERIAL1             1
#define HAS_WIRE0               1
#define SERIAL_INTERFACES_COUNT 2
#define WIRE_INTERFACES_COUNT   1

// Arduino pin names
// -----------------
#define PIN_D0  7u  // PA_7
#define PIN_D1  12u // PA_12
#define PIN_D2  3u  // PA_3
#define PIN_D3  10u // PA_10
#define PIN_D4  2u  // PA_2
#define PIN_D5  0u  // PA_0
#define PIN_D6  19u // PB_3
#define PIN_D7  11u // PA_11
#define PIN_D8  9u  // PA_9
#define PIN_D9  24u // PB_8
#define PIN_D10 25u // PB_9
#define PIN_D11 5u  // PA_5
#define PIN_D12 1u  // PA_1
#define PIN_A0  0u  // PA_0
#define PIN_A1  19u // PB_3
#define PIN_A2  1u  // PA_1

// Static pin names
// ----------------
static const unsigned char A0 = PIN_A0;
static const unsigned char A1 = PIN_A1;
static const unsigned char A2 = PIN_A2;
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
static const unsigned char D11 = PIN_D11;
static const unsigned char D12 = PIN_D12;

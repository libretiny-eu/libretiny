/* This file was auto-generated from ln-cb3s-v1.0.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         13 // Total GPIO count
#define NUM_DIGITAL_PINS   13 // Digital inputs/outputs
#define NUM_ANALOG_INPUTS  5  // ADC inputs
#define NUM_ANALOG_OUTPUTS 0  // PWM & DAC outputs
#define PINS_GPIO_MAX      25 // Last usable GPIO number

// Wire Interfaces
// ---------------
#define PIN_WIRE0_SCL_0  0u  // PA_0
#define PIN_WIRE0_SCL_1  1u  // PA_1
#define PIN_WIRE0_SCL_2  2u  // PA_2
#define PIN_WIRE0_SCL_3  3u  // PA_3
#define PIN_WIRE0_SCL_4  4u  // PA_4
#define PIN_WIRE0_SCL_5  5u  // PA_5
#define PIN_WIRE0_SCL_6  6u  // PA_6
#define PIN_WIRE0_SCL_7  9u  // PA_9
#define PIN_WIRE0_SCL_8  11u // PA_11
#define PIN_WIRE0_SCL_9  20u // PB_4
#define PIN_WIRE0_SCL_10 21u // PB_5
#define PIN_WIRE0_SCL_11 22u // PB_6
#define PIN_WIRE0_SCL_12 25u // PB_9
#define PIN_WIRE0_SDA_0  0u  // PA_0
#define PIN_WIRE0_SDA_1  1u  // PA_1
#define PIN_WIRE0_SDA_2  2u  // PA_2
#define PIN_WIRE0_SDA_3  3u  // PA_3
#define PIN_WIRE0_SDA_4  4u  // PA_4
#define PIN_WIRE0_SDA_5  5u  // PA_5
#define PIN_WIRE0_SDA_6  6u  // PA_6
#define PIN_WIRE0_SDA_7  9u  // PA_9
#define PIN_WIRE0_SDA_8  11u // PA_11
#define PIN_WIRE0_SDA_9  20u // PB_4
#define PIN_WIRE0_SDA_10 21u // PB_5
#define PIN_WIRE0_SDA_11 22u // PB_6
#define PIN_WIRE0_SDA_12 25u // PB_9
#define PINS_WIRE0_SCL   {0u, 1u, 2u, 3u, 4u, 5u, 6u, 9u, 11u, 20u, 21u, 22u, 25u}
#define PINS_WIRE0_SDA   {0u, 1u, 2u, 3u, 4u, 5u, 6u, 9u, 11u, 20u, 21u, 22u, 25u}

// Serial ports
// ------------
#define PIN_SERIAL0_RX  3u  // PA_3
#define PIN_SERIAL0_TX  2u  // PA_2
#define PIN_SERIAL1_TX  25u // PB_9
#define PINS_SERIAL0_RX {3u}
#define PINS_SERIAL0_TX {2u}
#define PINS_SERIAL1_TX {25u}

// Pin function macros
// -------------------
#define PIN_ADC2 0u  // PA_0
#define PIN_ADC3 1u  // PA_1
#define PIN_ADC4 4u  // PA_4
#define PIN_ADC6 20u // PB_4
#define PIN_ADC7 21u // PB_5
#define PIN_PA00 0u  // PA_0
#define PIN_PA01 1u  // PA_1
#define PIN_PA02 2u  // PA_2
#define PIN_PA03 3u  // PA_3
#define PIN_PA04 4u  // PA_4
#define PIN_PA05 5u  // PA_5
#define PIN_PA06 6u  // PA_6
#define PIN_PA09 9u  // PA_9
#define PIN_PA11 11u // PA_11
#define PIN_PB04 20u // PB_4
#define PIN_PB05 21u // PB_5
#define PIN_PB06 22u // PB_6
#define PIN_PB09 25u // PB_9
#define PIN_RX0  3u  // PA_3
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
#define PIN_D0  0u  // PA_0
#define PIN_D1  1u  // PA_1
#define PIN_D2  4u  // PA_4
#define PIN_D3  5u  // PA_5
#define PIN_D4  6u  // PA_6
#define PIN_D5  20u // PB_4
#define PIN_D6  25u // PB_9
#define PIN_D7  9u  // PA_9
#define PIN_D8  21u // PB_5
#define PIN_D9  22u // PB_6
#define PIN_D10 3u  // PA_3
#define PIN_D11 2u  // PA_2
#define PIN_D12 11u // PA_11
#define PIN_A0  0u  // PA_0
#define PIN_A1  1u  // PA_1
#define PIN_A2  4u  // PA_4
#define PIN_A3  20u // PB_4
#define PIN_A4  21u // PB_5

// Static pin names
// ----------------
static const unsigned char A0 = PIN_A0;
static const unsigned char A1 = PIN_A1;
static const unsigned char A2 = PIN_A2;
static const unsigned char A3 = PIN_A3;
static const unsigned char A4 = PIN_A4;
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

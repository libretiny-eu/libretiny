/* This file was auto-generated from ln-cb3s-v1.0.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         12 // Total GPIO count
#define NUM_DIGITAL_PINS   12 // Digital inputs/outputs
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
#define PIN_WIRE0_SCL_8  20u // PB_4
#define PIN_WIRE0_SCL_9  21u // PB_5
#define PIN_WIRE0_SCL_10 22u // PB_6
#define PIN_WIRE0_SCL_11 25u // PB_9
#define PIN_WIRE0_SDA_0  0u  // PA_0
#define PIN_WIRE0_SDA_1  1u  // PA_1
#define PIN_WIRE0_SDA_2  2u  // PA_2
#define PIN_WIRE0_SDA_3  3u  // PA_3
#define PIN_WIRE0_SDA_4  4u  // PA_4
#define PIN_WIRE0_SDA_5  5u  // PA_5
#define PIN_WIRE0_SDA_6  6u  // PA_6
#define PIN_WIRE0_SDA_7  9u  // PA_9
#define PIN_WIRE0_SDA_8  20u // PB_4
#define PIN_WIRE0_SDA_9  21u // PB_5
#define PIN_WIRE0_SDA_10 22u // PB_6
#define PIN_WIRE0_SDA_11 25u // PB_9
#define PINS_WIRE0_SCL   {0u, 1u, 2u, 3u, 4u, 5u, 6u, 9u, 20u, 21u, 22u, 25u}
#define PINS_WIRE0_SDA   {0u, 1u, 2u, 3u, 4u, 5u, 6u, 9u, 20u, 21u, 22u, 25u}

// Serial ports
// ------------
#define PIN_SERIAL0_RX  3u  // PA_3
#define PIN_SERIAL0_TX  2u  // PA_2
#define PIN_SERIAL1_RX  24u // PB_8 (not exposed, used internally by logger)
#define PIN_SERIAL1_TX  25u // PB_9
#define PINS_SERIAL0_RX {3u}
#define PINS_SERIAL0_TX {2u}
#define PINS_SERIAL1_RX {24u}
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
#define PIN_PB04 20u // PB_4
#define PIN_PB05 21u // PB_5
#define PIN_PB06 22u // PB_6
#define PIN_PB09 25u // PB_9
#define PIN_RX0  3u  // PA_3
#define PIN_RX1  24u // PB_8
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
#define PIN_D2  2u  // PA_2
#define PIN_D3  3u  // PA_3
#define PIN_D4  4u  // PA_4
#define PIN_D5  5u  // PA_5
#define PIN_D6  6u  // PA_6
#define PIN_D9  9u  // PA_9
#define PIN_D14 20u // PB_4
#define PIN_D15 21u // PB_5
#define PIN_D16 22u // PB_6
#define PIN_D19 25u // PB_9
#define PIN_A2  0u  // PA_0
#define PIN_A3  1u  // PA_1
#define PIN_A4  4u  // PA_4
#define PIN_A6  20u // PB_4
#define PIN_A7  21u // PB_5

// Static pin names
// ----------------
static const unsigned char A2  = PIN_A2;
static const unsigned char A3  = PIN_A3;
static const unsigned char A4  = PIN_A4;
static const unsigned char A6  = PIN_A6;
static const unsigned char A7  = PIN_A7;
static const unsigned char D0  = PIN_D0;
static const unsigned char D1  = PIN_D1;
static const unsigned char D2  = PIN_D2;
static const unsigned char D3  = PIN_D3;
static const unsigned char D4  = PIN_D4;
static const unsigned char D5  = PIN_D5;
static const unsigned char D6  = PIN_D6;
static const unsigned char D9  = PIN_D9;
static const unsigned char D14 = PIN_D14;
static const unsigned char D15 = PIN_D15;
static const unsigned char D16 = PIN_D16;
static const unsigned char D19 = PIN_D19;

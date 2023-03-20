/* This file was auto-generated from wr2.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         8  // Total GPIO count
#define NUM_DIGITAL_PINS   7  // Digital inputs/outputs
#define NUM_ANALOG_INPUTS  1  // ADC inputs
#define NUM_ANALOG_OUTPUTS 6  // PWM & DAC outputs
#define PINS_GPIO_MAX      41 // Last usable GPIO number

// Wire Interfaces
// ---------------
#define PIN_WIRE1_SCL 18u // PA_18
#define PIN_WIRE1_SDA 23u // PA_23

// Serial ports
// ------------
#define PIN_SERIAL0_RX 18u // PA_18
#define PIN_SERIAL0_TX 23u // PA_23

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
#define PIN_PWM0  14u // PA_14
#define PIN_PWM1  15u // PA_15
#define PIN_PWM2  0u  // PA_0
#define PIN_PWM3  12u // PA_12
#define PIN_PWM4  5u  // PA_5
#define PIN_RX0   18u // PA_18
#define PIN_SCK0  18u // PA_18
#define PIN_SCK1  18u // PA_18
#define PIN_SCL1  18u // PA_18
#define PIN_SDA1  23u // PA_23
#define PIN_TX0   23u // PA_23

// Port availability
// -----------------
#define HAS_SERIAL0             1
#define HAS_WIRE1               1
#define SERIAL_INTERFACES_COUNT 1
#define WIRE_INTERFACES_COUNT   1

// Arduino pin names
// -----------------
#define PIN_D0 12u // PA_12
#define PIN_D1 0u  // PA_0
#define PIN_D2 5u  // PA_5
#define PIN_D4 18u // PA_18
#define PIN_D5 23u // PA_23
#define PIN_D6 14u // PA_14
#define PIN_D7 15u // PA_15
#define PIN_A1 41u // AD_2

// Static pin names
// ----------------
static const unsigned char A1 = PIN_A1;
static const unsigned char D0 = PIN_D0;
static const unsigned char D1 = PIN_D1;
static const unsigned char D2 = PIN_D2;
static const unsigned char D4 = PIN_D4;
static const unsigned char D5 = PIN_D5;
static const unsigned char D6 = PIN_D6;
static const unsigned char D7 = PIN_D7;

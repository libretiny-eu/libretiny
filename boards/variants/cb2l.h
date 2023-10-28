/* This file was auto-generated from cb2l.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         9  // Total GPIO count
#define NUM_DIGITAL_PINS   9  // Digital inputs/outputs
#define NUM_ANALOG_INPUTS  0  // ADC inputs
#define NUM_ANALOG_OUTPUTS 5  // PWM & DAC outputs
#define PINS_GPIO_MAX      26 // Last usable GPIO number

// Serial ports
// ------------
#define PIN_SERIAL1_RX  10u // GPIO10
#define PIN_SERIAL1_TX  11u // GPIO11
#define PIN_SERIAL2_TX  0u  // GPIO0
#define PINS_SERIAL1_RX (pin_size_t[]){10u}
#define PINS_SERIAL1_TX (pin_size_t[]){11u}
#define PINS_SERIAL2_TX (pin_size_t[]){0u}

// Pin function macros
// -------------------
#define PIN_P0   0u  // GPIO0
#define PIN_P6   6u  // GPIO6
#define PIN_P7   7u  // GPIO7
#define PIN_P8   8u  // GPIO8
#define PIN_P10  10u // GPIO10
#define PIN_P11  11u // GPIO11
#define PIN_P21  21u // GPIO21
#define PIN_P24  24u // GPIO24
#define PIN_P26  26u // GPIO26
#define PIN_PWM0 6u  // GPIO6
#define PIN_PWM1 7u  // GPIO7
#define PIN_PWM2 8u  // GPIO8
#define PIN_PWM4 24u // GPIO24
#define PIN_PWM5 26u // GPIO26
#define PIN_RX1  10u // GPIO10
#define PIN_SCL2 0u  // GPIO0
#define PIN_SDA1 21u // GPIO21
#define PIN_TX1  11u // GPIO11
#define PIN_TX2  0u  // GPIO0

// Port availability
// -----------------
#define HAS_SERIAL1             1
#define HAS_SERIAL2             1
#define SERIAL_INTERFACES_COUNT 2

// Arduino pin names
// -----------------
#define PIN_D0 8u  // GPIO8
#define PIN_D1 7u  // GPIO7
#define PIN_D2 6u  // GPIO6
#define PIN_D3 26u // GPIO26
#define PIN_D4 24u // GPIO24
#define PIN_D5 10u // GPIO10
#define PIN_D6 0u  // GPIO0
#define PIN_D7 11u // GPIO11
#define PIN_D8 21u // GPIO21

// Static pin names
// ----------------
static const unsigned char D0 = PIN_D0;
static const unsigned char D1 = PIN_D1;
static const unsigned char D2 = PIN_D2;
static const unsigned char D3 = PIN_D3;
static const unsigned char D4 = PIN_D4;
static const unsigned char D5 = PIN_D5;
static const unsigned char D6 = PIN_D6;
static const unsigned char D7 = PIN_D7;
static const unsigned char D8 = PIN_D8;

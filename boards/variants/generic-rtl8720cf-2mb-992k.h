/* This file was auto-generated from generic-rtl8720cf-2mb-992k.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         20 // Total GPIO count
#define NUM_DIGITAL_PINS   20 // Digital inputs/outputs
#define NUM_ANALOG_INPUTS  0  // ADC inputs
#define NUM_ANALOG_OUTPUTS 16 // PWM & DAC outputs
#define PINS_GPIO_MAX      23 // Last usable GPIO number

// SPI Interfaces
// --------------
#define PIN_SPI0_CS_0   2u  // PIN_A2
#define PIN_SPI0_CS_1   7u  // PIN_A7
#define PIN_SPI0_CS_2   15u // PIN_A15
#define PIN_SPI0_MISO_0 10u // PIN_A10
#define PIN_SPI0_MISO_1 20u // PIN_A20
#define PIN_SPI0_MOSI_0 4u  // PIN_A4
#define PIN_SPI0_MOSI_1 9u  // PIN_A9
#define PIN_SPI0_MOSI_2 19u // PIN_A19
#define PIN_SPI0_SCK_0  3u  // PIN_A3
#define PIN_SPI0_SCK_1  8u  // PIN_A8
#define PIN_SPI0_SCK_2  16u // PIN_A16
#define PINS_SPI0_CS    (pin_size_t[]){2u, 7u, 15u}
#define PINS_SPI0_MISO  (pin_size_t[]){10u, 20u}
#define PINS_SPI0_MOSI  (pin_size_t[]){4u, 9u, 19u}
#define PINS_SPI0_SCK   (pin_size_t[]){3u, 8u, 16u}

// Wire Interfaces
// ---------------
#define PIN_WIRE0_SCL_0 2u  // PIN_A2
#define PIN_WIRE0_SCL_1 11u // PIN_A11
#define PIN_WIRE0_SCL_2 15u // PIN_A15
#define PIN_WIRE0_SCL_3 19u // PIN_A19
#define PIN_WIRE0_SDA_0 3u  // PIN_A3
#define PIN_WIRE0_SDA_1 12u // PIN_A12
#define PIN_WIRE0_SDA_2 16u // PIN_A16
#define PIN_WIRE0_SDA_3 20u // PIN_A20
#define PINS_WIRE0_SCL  (pin_size_t[]){2u, 11u, 15u, 19u}
#define PINS_WIRE0_SDA  (pin_size_t[]){3u, 12u, 16u, 20u}

// Serial ports
// ------------
#define PIN_SERIAL0_CTS  10u // PIN_A10
#define PIN_SERIAL0_RTS  9u  // PIN_A9
#define PIN_SERIAL0_RX_0 12u // PIN_A12
#define PIN_SERIAL0_RX_1 13u // PIN_A13
#define PIN_SERIAL0_TX_0 11u // PIN_A11
#define PIN_SERIAL0_TX_1 14u // PIN_A14
#define PIN_SERIAL1_CTS  4u  // PIN_A4
#define PIN_SERIAL1_RX_0 0u  // PIN_A0
#define PIN_SERIAL1_RX_1 2u  // PIN_A2
#define PIN_SERIAL1_TX_0 1u  // PIN_A1
#define PIN_SERIAL1_TX_1 3u  // PIN_A3
#define PIN_SERIAL2_CTS  19u // PIN_A19
#define PIN_SERIAL2_RTS  20u // PIN_A20
#define PIN_SERIAL2_RX   15u // PIN_A15
#define PIN_SERIAL2_TX   16u // PIN_A16
#define PINS_SERIAL0_CTS (pin_size_t[]){10u}
#define PINS_SERIAL0_RTS (pin_size_t[]){9u}
#define PINS_SERIAL0_RX  (pin_size_t[]){12u, 13u}
#define PINS_SERIAL0_TX  (pin_size_t[]){11u, 14u}
#define PINS_SERIAL1_CTS (pin_size_t[]){4u}
#define PINS_SERIAL1_RX  (pin_size_t[]){0u, 2u}
#define PINS_SERIAL1_TX  (pin_size_t[]){1u, 3u}
#define PINS_SERIAL2_CTS (pin_size_t[]){19u}
#define PINS_SERIAL2_RTS (pin_size_t[]){20u}
#define PINS_SERIAL2_RX  (pin_size_t[]){15u}
#define PINS_SERIAL2_TX  (pin_size_t[]){16u}

// Pin function macros
// -------------------
#define PIN_CS0   15u // PIN_A15
#define PIN_CTS0  10u // PIN_A10
#define PIN_CTS1  4u  // PIN_A4
#define PIN_CTS2  19u // PIN_A19
#define PIN_MOSI0 19u // PIN_A19
#define PIN_PA00  0u  // PIN_A0
#define PIN_PA01  1u  // PIN_A1
#define PIN_PA02  2u  // PIN_A2
#define PIN_PA03  3u  // PIN_A3
#define PIN_PA04  4u  // PIN_A4
#define PIN_PA07  7u  // PIN_A7
#define PIN_PA08  8u  // PIN_A8
#define PIN_PA09  9u  // PIN_A9
#define PIN_PA10  10u // PIN_A10
#define PIN_PA11  11u // PIN_A11
#define PIN_PA12  12u // PIN_A12
#define PIN_PA13  13u // PIN_A13
#define PIN_PA14  14u // PIN_A14
#define PIN_PA15  15u // PIN_A15
#define PIN_PA16  16u // PIN_A16
#define PIN_PA17  17u // PIN_A17
#define PIN_PA18  18u // PIN_A18
#define PIN_PA19  19u // PIN_A19
#define PIN_PA20  20u // PIN_A20
#define PIN_PA23  23u // PIN_A23
#define PIN_PWM0  20u // PIN_A20
#define PIN_PWM5  17u // PIN_A17
#define PIN_PWM6  18u // PIN_A18
#define PIN_PWM7  23u // PIN_A23
#define PIN_RTS0  9u  // PIN_A9
#define PIN_RTS2  20u // PIN_A20
#define PIN_RX2   15u // PIN_A15
#define PIN_SCK0  16u // PIN_A16
#define PIN_TX2   16u // PIN_A16

// Port availability
// -----------------
#define HAS_SERIAL0             1
#define HAS_SERIAL1             1
#define HAS_SERIAL2             1
#define HAS_SPI0                1
#define HAS_WIRE0               1
#define SERIAL_INTERFACES_COUNT 3
#define SPI_INTERFACES_COUNT    1
#define WIRE_INTERFACES_COUNT   1

// Arduino pin names
// -----------------
#define PIN_D0  0u  // PIN_A0
#define PIN_D1  1u  // PIN_A1
#define PIN_D2  2u  // PIN_A2
#define PIN_D3  3u  // PIN_A3
#define PIN_D4  4u  // PIN_A4
#define PIN_D5  7u  // PIN_A7
#define PIN_D6  8u  // PIN_A8
#define PIN_D7  9u  // PIN_A9
#define PIN_D8  10u // PIN_A10
#define PIN_D9  11u // PIN_A11
#define PIN_D10 12u // PIN_A12
#define PIN_D11 13u // PIN_A13
#define PIN_D12 14u // PIN_A14
#define PIN_D13 15u // PIN_A15
#define PIN_D14 16u // PIN_A16
#define PIN_D15 17u // PIN_A17
#define PIN_D16 18u // PIN_A18
#define PIN_D17 19u // PIN_A19
#define PIN_D18 20u // PIN_A20
#define PIN_D19 23u // PIN_A23

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
static const unsigned char D9 = PIN_D9;
static const unsigned char D10 = PIN_D10;
static const unsigned char D11 = PIN_D11;
static const unsigned char D12 = PIN_D12;
static const unsigned char D13 = PIN_D13;
static const unsigned char D14 = PIN_D14;
static const unsigned char D15 = PIN_D15;
static const unsigned char D16 = PIN_D16;
static const unsigned char D17 = PIN_D17;
static const unsigned char D18 = PIN_D18;
static const unsigned char D19 = PIN_D19;

/* This file was auto-generated from generic-bk7252.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         38 // Total GPIO count
#define NUM_DIGITAL_PINS   38 // Digital inputs/outputs
#define NUM_ANALOG_INPUTS  7  // ADC inputs
#define NUM_ANALOG_OUTPUTS 4  // PWM & DAC outputs
#define PINS_GPIO_MAX      39 // Last usable GPIO number

// SPI Interfaces
// --------------
#define PIN_SPI0_CS    15u // GPIO15
#define PIN_SPI0_MISO  17u // GPIO17
#define PIN_SPI0_MOSI  16u // GPIO16
#define PIN_SPI0_SCK   14u // GPIO14
#define PINS_SPI0_CS   (pin_size_t[]){15u}
#define PINS_SPI0_MISO (pin_size_t[]){17u}
#define PINS_SPI0_MOSI (pin_size_t[]){16u}
#define PINS_SPI0_SCK  (pin_size_t[]){14u}

// Wire Interfaces
// ---------------
#define PIN_WIRE1_SCL  20u // GPIO20
#define PIN_WIRE1_SDA  21u // GPIO21
#define PIN_WIRE2_SCL  0u  // GPIO0
#define PIN_WIRE2_SDA  1u  // GPIO1
#define PINS_WIRE1_SCL (pin_size_t[]){20u}
#define PINS_WIRE1_SDA (pin_size_t[]){21u}
#define PINS_WIRE2_SCL (pin_size_t[]){0u}
#define PINS_WIRE2_SDA (pin_size_t[]){1u}

// Serial ports
// ------------
#define PIN_SERIAL1_CTS  12u // GPIO12
#define PIN_SERIAL1_RTS  13u // GPIO13
#define PIN_SERIAL1_RX   10u // GPIO10
#define PIN_SERIAL1_TX   11u // GPIO11
#define PIN_SERIAL2_RX   1u  // GPIO1
#define PIN_SERIAL2_TX   0u  // GPIO0
#define PINS_SERIAL1_CTS (pin_size_t[]){12u}
#define PINS_SERIAL1_RTS (pin_size_t[]){13u}
#define PINS_SERIAL1_RX  (pin_size_t[]){10u}
#define PINS_SERIAL1_TX  (pin_size_t[]){11u}
#define PINS_SERIAL2_RX  (pin_size_t[]){1u}
#define PINS_SERIAL2_TX  (pin_size_t[]){0u}

// Pin function macros
// -------------------
#define PIN_ADC1 4u  // GPIO4
#define PIN_ADC2 5u  // GPIO5
#define PIN_ADC3 23u // GPIO23
#define PIN_ADC4 2u  // GPIO2
#define PIN_ADC5 3u  // GPIO3
#define PIN_ADC6 12u // GPIO12
#define PIN_ADC7 13u // GPIO13
#define PIN_CS   15u // GPIO15
#define PIN_CTS1 12u // GPIO12
#define PIN_MISO 17u // GPIO17
#define PIN_MOSI 16u // GPIO16
#define PIN_P0   0u  // GPIO0
#define PIN_P1   1u  // GPIO1
#define PIN_P2   2u  // GPIO2
#define PIN_P3   3u  // GPIO3
#define PIN_P4   4u  // GPIO4
#define PIN_P5   5u  // GPIO5
#define PIN_P6   6u  // GPIO6
#define PIN_P7   7u  // GPIO7
#define PIN_P10  10u // GPIO10
#define PIN_P11  11u // GPIO11
#define PIN_P12  12u // GPIO12
#define PIN_P13  13u // GPIO13
#define PIN_P14  14u // GPIO14
#define PIN_P15  15u // GPIO15
#define PIN_P16  16u // GPIO16
#define PIN_P17  17u // GPIO17
#define PIN_P18  18u // GPIO18
#define PIN_P19  19u // GPIO19
#define PIN_P20  20u // GPIO20
#define PIN_P21  21u // GPIO21
#define PIN_P22  22u // GPIO22
#define PIN_P23  23u // GPIO23
#define PIN_P24  24u // GPIO24
#define PIN_P25  25u // GPIO25
#define PIN_P26  26u // GPIO26
#define PIN_P27  27u // GPIO27
#define PIN_P28  28u // GPIO28
#define PIN_P29  29u // GPIO29
#define PIN_P30  30u // GPIO30
#define PIN_P31  31u // GPIO31
#define PIN_P32  32u // GPIO32
#define PIN_P33  33u // GPIO33
#define PIN_P34  34u // GPIO34
#define PIN_P35  35u // GPIO35
#define PIN_P36  36u // GPIO36
#define PIN_P37  37u // GPIO37
#define PIN_P38  38u // GPIO38
#define PIN_P39  39u // GPIO39
#define PIN_PWM0 6u  // GPIO6
#define PIN_PWM1 7u  // GPIO7
#define PIN_PWM4 24u // GPIO24
#define PIN_PWM5 26u // GPIO26
#define PIN_RTS1 13u // GPIO13
#define PIN_RX1  10u // GPIO10
#define PIN_RX2  1u  // GPIO1
#define PIN_SCK  14u // GPIO14
#define PIN_SCL1 20u // GPIO20
#define PIN_SCL2 0u  // GPIO0
#define PIN_SDA1 21u // GPIO21
#define PIN_SDA2 1u  // GPIO1
#define PIN_TX1  11u // GPIO11
#define PIN_TX2  0u  // GPIO0

// Port availability
// -----------------
#define HAS_SERIAL1             1
#define HAS_SERIAL2             1
#define HAS_SPI0                1
#define HAS_WIRE1               1
#define HAS_WIRE2               1
#define SERIAL_INTERFACES_COUNT 2
#define SPI_INTERFACES_COUNT    1
#define WIRE_INTERFACES_COUNT   2

// Arduino pin names
// -----------------
#define PIN_D0  0u  // GPIO0
#define PIN_D1  1u  // GPIO1
#define PIN_D2  2u  // GPIO2
#define PIN_D3  3u  // GPIO3
#define PIN_D4  4u  // GPIO4
#define PIN_D5  5u  // GPIO5
#define PIN_D6  6u  // GPIO6
#define PIN_D7  7u  // GPIO7
#define PIN_D8  10u // GPIO10
#define PIN_D9  11u // GPIO11
#define PIN_D10 12u // GPIO12
#define PIN_D11 13u // GPIO13
#define PIN_D12 14u // GPIO14
#define PIN_D13 15u // GPIO15
#define PIN_D14 16u // GPIO16
#define PIN_D15 17u // GPIO17
#define PIN_D16 18u // GPIO18
#define PIN_D17 19u // GPIO19
#define PIN_D18 20u // GPIO20
#define PIN_D19 21u // GPIO21
#define PIN_D20 22u // GPIO22
#define PIN_D21 23u // GPIO23
#define PIN_D22 24u // GPIO24
#define PIN_D23 25u // GPIO25
#define PIN_D24 26u // GPIO26
#define PIN_D25 27u // GPIO27
#define PIN_D26 28u // GPIO28
#define PIN_D27 29u // GPIO29
#define PIN_D28 30u // GPIO30
#define PIN_D29 31u // GPIO31
#define PIN_D30 32u // GPIO32
#define PIN_D31 33u // GPIO33
#define PIN_D32 34u // GPIO34
#define PIN_D33 35u // GPIO35
#define PIN_D34 36u // GPIO36
#define PIN_D35 37u // GPIO37
#define PIN_D36 38u // GPIO38
#define PIN_D37 39u // GPIO39
#define PIN_A1  4u  // GPIO4
#define PIN_A2  5u  // GPIO5
#define PIN_A3  23u // GPIO23
#define PIN_A4  3u  // GPIO3
#define PIN_A5  2u  // GPIO2
#define PIN_A6  12u // GPIO12
#define PIN_A7  13u // GPIO13

// Static pin names
// ----------------
static const unsigned char A1 = PIN_A1;
static const unsigned char A2 = PIN_A2;
static const unsigned char A3 = PIN_A3;
static const unsigned char A4 = PIN_A4;
static const unsigned char A5 = PIN_A5;
static const unsigned char A6 = PIN_A6;
static const unsigned char A7 = PIN_A7;
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
static const unsigned char D20 = PIN_D20;
static const unsigned char D21 = PIN_D21;
static const unsigned char D22 = PIN_D22;
static const unsigned char D23 = PIN_D23;
static const unsigned char D24 = PIN_D24;
static const unsigned char D25 = PIN_D25;
static const unsigned char D26 = PIN_D26;
static const unsigned char D27 = PIN_D27;
static const unsigned char D28 = PIN_D28;
static const unsigned char D29 = PIN_D29;
static const unsigned char D30 = PIN_D30;
static const unsigned char D31 = PIN_D31;
static const unsigned char D32 = PIN_D32;
static const unsigned char D33 = PIN_D33;
static const unsigned char D34 = PIN_D34;
static const unsigned char D35 = PIN_D35;
static const unsigned char D36 = PIN_D36;
static const unsigned char D37 = PIN_D37;

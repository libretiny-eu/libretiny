/* This file was auto-generated from generic-bk7252.json using boardgen */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         38
#define NUM_DIGITAL_PINS   38
#define NUM_ANALOG_INPUTS  7
#define NUM_ANALOG_OUTPUTS 0

// Analog pins
// -----------
#define PIN_A5 2u     // GPIO2
#define PIN_A4 3u     // GPIO3
#define PIN_A1 4u     // GPIO4
#define PIN_A2 5u     // GPIO5
#define PIN_A6 10u    // GPIO12
#define PIN_A7 11u    // GPIO13
#define PIN_A3 21u    // GPIO23
#define A5     PIN_A5
#define A4     PIN_A4
#define A1     PIN_A1
#define A2     PIN_A2
#define A6     PIN_A6
#define A7     PIN_A7
#define A3     PIN_A3

// SPI Interfaces
// --------------
#define SPI_INTERFACES_COUNT 0

// Wire Interfaces
// ---------------
#define WIRE_INTERFACES_COUNT 2
#define PIN_WIRE1_SCL         18u // GPIO20
#define PIN_WIRE1_SDA         19u // GPIO21
#define PIN_WIRE2_SCL         0u  // GPIO0
#define PIN_WIRE2_SDA         1u  // GPIO1

// Serial ports
// ------------
#define SERIAL_INTERFACES_COUNT 2
#define PIN_SERIAL1_CTS         10u // GPIO12
#define PIN_SERIAL1_RTS         11u // GPIO13
#define PIN_SERIAL1_RX          8u  // GPIO10
#define PIN_SERIAL1_TX          9u  // GPIO11
#define PIN_SERIAL2_RX          1u  // GPIO1
#define PIN_SERIAL2_TX          0u  // GPIO0

// Pin function macros
// -------------------
#define PIN_ADC1 4u  // GPIO4
#define PIN_ADC2 5u  // GPIO5
#define PIN_ADC3 21u // GPIO23
#define PIN_ADC4 2u  // GPIO2
#define PIN_ADC5 3u  // GPIO3
#define PIN_ADC6 10u // GPIO12
#define PIN_ADC7 11u // GPIO13
#define PIN_CS   13u // GPIO15
#define PIN_CTS1 10u // GPIO12
#define PIN_MISO 15u // GPIO17
#define PIN_MOSI 14u // GPIO16
#define PIN_P0   0u  // GPIO0
#define PIN_P1   1u  // GPIO1
#define PIN_P2   2u  // GPIO2
#define PIN_P3   3u  // GPIO3
#define PIN_P4   4u  // GPIO4
#define PIN_P5   5u  // GPIO5
#define PIN_P6   6u  // GPIO6
#define PIN_P7   7u  // GPIO7
#define PIN_P10  8u  // GPIO10
#define PIN_P11  9u  // GPIO11
#define PIN_P12  10u // GPIO12
#define PIN_P13  11u // GPIO13
#define PIN_P14  12u // GPIO14
#define PIN_P15  13u // GPIO15
#define PIN_P16  14u // GPIO16
#define PIN_P17  15u // GPIO17
#define PIN_P18  16u // GPIO18
#define PIN_P19  17u // GPIO19
#define PIN_P20  18u // GPIO20
#define PIN_P21  19u // GPIO21
#define PIN_P22  20u // GPIO22
#define PIN_P23  21u // GPIO23
#define PIN_P24  22u // GPIO24
#define PIN_P25  23u // GPIO25
#define PIN_P26  24u // GPIO26
#define PIN_P27  25u // GPIO27
#define PIN_P28  26u // GPIO28
#define PIN_P29  27u // GPIO29
#define PIN_P30  28u // GPIO30
#define PIN_P31  29u // GPIO31
#define PIN_P32  30u // GPIO32
#define PIN_P33  31u // GPIO33
#define PIN_P34  32u // GPIO34
#define PIN_P35  33u // GPIO35
#define PIN_P36  34u // GPIO36
#define PIN_P37  35u // GPIO37
#define PIN_P38  36u // GPIO38
#define PIN_P39  37u // GPIO39
#define PIN_PWM0 6u  // GPIO6
#define PIN_PWM1 7u  // GPIO7
#define PIN_PWM4 22u // GPIO24
#define PIN_PWM5 24u // GPIO26
#define PIN_RTS1 11u // GPIO13
#define PIN_RX1  8u  // GPIO10
#define PIN_RX2  1u  // GPIO1
#define PIN_SCK  12u // GPIO14
#define PIN_SCL1 18u // GPIO20
#define PIN_SCL2 0u  // GPIO0
#define PIN_SDA1 19u // GPIO21
#define PIN_SDA2 1u  // GPIO1
#define PIN_TX1  9u  // GPIO11
#define PIN_TX2  0u  // GPIO0

// Port availability
// -----------------
#define HAS_SERIAL1 1
#define HAS_SERIAL2 1
#define HAS_WIRE1   1
#define HAS_WIRE2   1

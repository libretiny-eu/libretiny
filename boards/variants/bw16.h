/* BW16 (RTL8720DN) — pin roles from the official Realtek Arduino variant
 * (ameba-arduino-d, rtl8720dn_bw16). PA_n = n, PB_n = 32 + n. */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         13 // Total GPIO count
#define NUM_DIGITAL_PINS   13 // Digital inputs/outputs
#define NUM_ANALOG_INPUTS  3  // ADC inputs
#define NUM_ANALOG_OUTPUTS 0  // PWM & DAC outputs
#define PINS_GPIO_MAX      42 // Last usable GPIO number

// Analog pins
// -----------
#define PIN_A0	 33u // PB1
#define PIN_A1	 34u // PB2
#define PIN_A2	 35u // PB3
#define PIN_ADC0 33u // PB1
#define PIN_ADC1 34u // PB2
#define PIN_ADC2 35u // PB3
#define PINS_ADC {33u, 34u, 35u}

// Serial ports
// ------------
// LOG_UART on PA7 (TX) / PA8 (RX) is the console and download port; UART1 goes to the BT controller.
#define HAS_SERIAL0 1
#define HAS_SERIAL2 1

#define PIN_SERIAL0_TX 7u  // PA7
#define PIN_SERIAL0_RX 8u  // PA8
#define PINS_SERIAL0_TX {7u}
#define PINS_SERIAL0_RX {8u}
// UART2 (user, "TX0/RX0" silk) on PB1 (TX) / PB2 (RX).
#define PIN_SERIAL2_TX 33u // PB1
#define PIN_SERIAL2_RX 34u // PB2
#define PINS_SERIAL2_TX {33u}
#define PINS_SERIAL2_RX {34u}

#define SERIAL_INTERFACES_COUNT 2

// Pin names
// ---------
#define PIN_PA7  7u
#define PIN_PA8  8u
#define PIN_PA12 12u
#define PIN_PA13 13u
#define PIN_PA14 14u
#define PIN_PA15 15u
#define PIN_PA25 25u
#define PIN_PA26 26u
#define PIN_PA27 27u
#define PIN_PA30 30u
#define PIN_PB1  33u
#define PIN_PB2  34u
#define PIN_PB3  35u

// Pin function macros
// -------------------
#define PINS_GPIO {7u, 8u, 12u, 13u, 14u, 15u, 25u, 26u, 27u, 30u, 33u, 34u, 35u}

// LED
// ---
// Onboard RGB LED: R=PA12, G=PA14, B=PA13 (PA14 has no PWM mux).
#define PIN_LED_R 12u
#define PIN_LED_G 14u
#define PIN_LED_B 13u
#define PIN_LED	  12u // PIN_LED_R
#define LED_BUILTIN PIN_LED

// clang-format on

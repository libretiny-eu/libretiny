/* This file is hand-written for the Enel X JuiceBox 40 (WGM160P host).
 *
 * boardgen 0.12.0 cannot autogenerate the EFM32 variant: its pin model derives
 * the pin number as re.sub(r"\D","",GPIO_name), so PA4/PB4/PD4/PE4 all collapse
 * to "4" (EFM32 needs port-aware encoding), and its Arduino label field 'ARD'
 * only accepts D0/D1/.../A0/A1 (LibreTiny board-specific names like LED0/BTN0
 * are silently dropped). Both gaps are upstream-boardgen work.
 *
 * Pin roles come from the bench-confirmed Gecko OS GPIO usage map of a live
 * JuiceBox 40 (probed via gpio_get/gpio_set over the unauthenticated runtime
 * shell, 2026-05). The JuiceBox repurposes PB5/PB6 away from the module's
 * nominal UART flow-control roles to RGB LED channels. Colors bench-verified
 * 2026-06-11 by driving each pin ALONE via SWD with an observer:
 * PB5=red, PB6=green, PD8=blue, all ACTIVE-HIGH. The earlier map
 * (PB3=red, PB5=green) was a one-slot perceptual shift from a cycling sketch
 * and is WRONG: PB3 drives nothing visible (the Gecko OS table labels it RGB,
 * but it has no visible function), so PB3 is exposed only as a plain GPIO.
 * PD8 was found by a port-by-port GPIO sweep — it is NOT in the Gecko OS GPIO
 * usage map (that trace was incomplete).
 *
 * EFM32 pin encoding: (port_index << 4) | pin_number
 *   port A=0, B=1, C=2, D=3, E=4, F=5
 *   PB5=0x15 PB6=0x16 PD8=0x38 PC4=0x24 PE6=0x46 PE7=0x47 PB3=0x13 PF11=0x5B
 * This matches cores/silabs-efm32gg11/arduino/src/ArduinoFamily.h.
 */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         7    // Arduino-named pins exposed on the JuiceBox host
#define NUM_DIGITAL_PINS   7    // Digital inputs/outputs
#define NUM_ANALOG_INPUTS  0    // ADC inputs (none mapped in Phase 1)
#define NUM_ANALOG_OUTPUTS 3    // PWM on the 3 RGB LED pins (PB5/PB6/PD8) via analogWrite
#define PINS_GPIO_MAX      0x5B // Last usable encoded GPIO number (PF11, ATmega RESET)

// Serial ports
// ------------
// USART0 LOC1 routes to the WGM160P console UART (module pins 46/47).
#define PIN_SERIAL0_RX  0x46u // PE6
#define PIN_SERIAL0_TX  0x47u // PE7
#define PINS_SERIAL0_RX {0x46u}
#define PINS_SERIAL0_TX {0x47u}

// Pin function macros
// -------------------
#define PIN_PB5 0x15u // PB5 (RGB LED red, active-high)
#define PIN_PB6 0x16u // PB6 (RGB LED green, active-high)
#define PIN_PD8 0x38u // PD8 (RGB LED blue, active-high)
#define PIN_PC4 0x24u // PC4 (factory_reset line)
#define PIN_PE6 0x46u // PE6 (USART0 LOC1 RX)
#define PIN_PE7 0x47u // PE7 (USART0 LOC1 TX)
#define PIN_PB3 0x13u // PB3 (no visible function; Gecko OS table said RGB)
#define PIN_PF11 0x5Bu // PF11 (ATmega328P EVSE-controller RESET, active-low)
#define PIN_RX  0x46u // PE6
#define PIN_TX  0x47u // PE7

// Port availability
// -----------------
#define HAS_SERIAL0             1
#define SERIAL_INTERFACES_COUNT 1

// Arduino pin names
// -----------------
#define PIN_D0 0x15u // PB5 — RGB LED red (bench-verified 2026-06-11)
#define PIN_D1 0x16u // PB6 — RGB LED green (bench-verified 2026-06-11)
#define PIN_D2 0x38u // PD8 — RGB LED blue (SWD sweep find; not in Gecko OS map)
#define PIN_D3 0x24u // PC4 — factory_reset line (Gecko GPIO 16)
#define PIN_D4 0x46u // PE6 — console UART RX (USART0 LOC1)
#define PIN_D5 0x47u // PE7 — console UART TX (USART0 LOC1)
#define PIN_D6 0x13u // PB3 — no visible function (Gecko OS table said RGB; drives nothing)

// Board-specific Arduino names
// ----------------------------
#define LED0        PIN_D0
#define LED1        PIN_D1
#define LED2        PIN_D2
#define LED_R       PIN_D0 // PB5 — bench-verified red, active-high
#define LED_G       PIN_D1 // PB6 — bench-verified green, active-high
#define LED_B       PIN_D2 // PD8 — bench-verified blue, active-high
#define BTN0        PIN_D3
#define EVSE_RESET  PIN_PF11 // PF11 — ATmega328P RESET, active-low (HIGH=run, LOW=reset; continuity-confirmed 2026-06-13)
#define LED_BUILTIN PIN_D0

// Static pin names
// ----------------
static const unsigned char D0 = PIN_D0;
static const unsigned char D1 = PIN_D1;
static const unsigned char D2 = PIN_D2;
static const unsigned char D3 = PIN_D3;
static const unsigned char D4 = PIN_D4;
static const unsigned char D5 = PIN_D5;
static const unsigned char D6 = PIN_D6;

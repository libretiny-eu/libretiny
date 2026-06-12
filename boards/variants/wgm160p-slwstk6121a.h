/* This file is hand-written for the WGM160P SLWSTK6121A board.
 *
 * boardgen 0.12.0 now accepts our board JSON (after adding upload.flash_size
 * and renaming our silkscreen field from 'PCB' to the schema's 'PHYSICAL'),
 * but the generated variant is degenerate (PINS_COUNT=0, no .c emitted)
 * because boardgen's pin model assumes:
 *   - the pin number is `re.sub(r"\D","",GPIO_name)` — so PA4 collides with
 *     PB4/PD4/PE4 (all -> "4"). EFM32 needs port-aware encoding.
 *   - the Arduino label field is 'ARD' with values D0/D1/.../A0/A1/...
 *     ('LED0', 'BTN0', etc. are silently dropped); the LibreTiny convention
 *     for board-specific names (LED0, BTN0) doesn't fit boardgen's model.
 *
 * Both gaps are upstream-boardgen work, separate from the LibreTiny PR.
 *
 * EFM32 pin encoding used below: (port_index << 4) | pin_number
 *   port A=0, B=1, C=2, D=3, E=4, F=5
 *   PA4=0x04 PA5=0x05 PD6=0x36 PD8=0x38 PE6=0x46 PE7=0x47
 * This matches the encoding cores/silabs-efm32gg11/arduino/src/ArduinoFamily.h
 * uses to unpack into emlib GPIO_Port_TypeDef + pin pairs.
 */

#pragma once

// clang-format off

// Pins
// ----
#define PINS_COUNT         6    // Broken-out, Arduino-named pins on the WSTK
#define NUM_DIGITAL_PINS   6    // Digital inputs/outputs
#define NUM_ANALOG_INPUTS  0    // ADC inputs (none wired on the WSTK header in Phase 1)
#define NUM_ANALOG_OUTPUTS 0    // PWM & DAC outputs (none in Phase 1)
#define PINS_GPIO_MAX      0x47 // Last usable encoded GPIO number (PE7)

// Serial ports
// ------------
// USART0 LOC1 routes to the on-board VCOM (J-Link CDC-ACM) bridge.
#define PIN_SERIAL0_RX  0x46u // PE6
#define PIN_SERIAL0_TX  0x47u // PE7
#define PINS_SERIAL0_RX {0x46u}
#define PINS_SERIAL0_TX {0x47u}

// Pin function macros
// -------------------
#define PIN_PA4 0x04u // PA4
#define PIN_PA5 0x05u // PA5
#define PIN_PD6 0x36u // PD6
#define PIN_PD8 0x38u // PD8
#define PIN_PE6 0x46u // PE6 (USART0 LOC1 RX)
#define PIN_PE7 0x47u // PE7 (USART0 LOC1 TX)
#define PIN_RX  0x46u // PE6
#define PIN_TX  0x47u // PE7

// Port availability
// -----------------
#define HAS_SERIAL0             1
#define SERIAL_INTERFACES_COUNT 1

// Arduino pin names
// -----------------
#define PIN_D0 0x04u // PA4 — WSTK LED0 (active-high)
#define PIN_D1 0x05u // PA5 — WSTK LED1 (active-high)
#define PIN_D2 0x36u // PD6 — WSTK BTN0 (active-low)
#define PIN_D3 0x38u // PD8 — WSTK BTN1 (active-low)
#define PIN_D4 0x46u // PE6 — VCOM RX (USART0 LOC1)
#define PIN_D5 0x47u // PE7 — VCOM TX (USART0 LOC1)

// Board-specific Arduino names
// ----------------------------
#define LED0        PIN_D0
#define LED1        PIN_D1
#define BTN0        PIN_D2
#define BTN1        PIN_D3
#define LED_BUILTIN PIN_D0

// Static pin names
// ----------------
static const unsigned char D0 = PIN_D0;
static const unsigned char D1 = PIN_D1;
static const unsigned char D2 = PIN_D2;
static const unsigned char D3 = PIN_D3;
static const unsigned char D4 = PIN_D4;
static const unsigned char D5 = PIN_D5;

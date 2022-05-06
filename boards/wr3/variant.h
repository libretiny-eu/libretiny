/* Copyright (c) Kuba Szczodrzyński 2022-04-23. */

#pragma once

#include <WVariant.h>

// Pins
// ----

// Number of pins defined in PinDescription array
#define PINS_COUNT		   (12u)
#define NUM_DIGITAL_PINS   (12u)
#define NUM_ANALOG_INPUTS  (2u)
#define NUM_ANALOG_OUTPUTS (0u)

// Low-level pin register query macros
// -----------------------------------
#define portOutputRegister(P) ((volatile uint32_t *)(0x40001000 + (P)*0x0C))
#define portInputRegister(P)  ((volatile uint32_t *)(0x40001050 + (P)*4))
#define portModeRegister(P)	  ((volatile uint32_t *)(0x40001004 + (P)*0x0C))

// LEDs
// ----
#define PIN_LED		(4u)
#define LED_BUILTIN PIN_LED

// Analog pins
// -----------
#define PIN_A0 (1u) // PA_19
#define PIN_A1 (0u) // dummy, this is NOT usable
#define PIN_A2 (5u) // PA_20

static const uint8_t A0 = PIN_A0;
static const uint8_t A1 = PIN_A1;
static const uint8_t A2 = PIN_A2;

// SPI Interfaces
// --------------
#define SPI_INTERFACES_COUNT 0

// Wire Interfaces
// ---------------
#define WIRE_INTERFACES_COUNT 0

// Serial ports
// ------------
#ifdef __cplusplus
#include "LOGUARTClass.h"
extern LOGUARTClass Serial;
// LOGUARTClass
#define PIN_SERIAL2_RX PA_29
#define PIN_SERIAL2_TX PA_30
// UARTClassOne
#define PIN_SERIAL0_RX PA_18
#define PIN_SERIAL0_TX PA_23
#endif // __cplusplus

#define SERIAL_PORT_USBVIRTUAL	  Serial
#define SERIAL_PORT_MONITOR		  Serial
#define SERIAL_PORT_HARDWARE	  Serial
#define SERIAL_PORT_HARDWARE_OPEN Serial

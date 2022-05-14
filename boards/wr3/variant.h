/* Copyright (c) Kuba Szczodrzyński 2022-04-23. */

#pragma once

#include <WVariant.h>

// Pins
// ----

// Number of pins defined in PinDescription array
#define PINS_COUNT		   12
#define NUM_DIGITAL_PINS   11
#define NUM_ANALOG_INPUTS  2
#define NUM_ANALOG_OUTPUTS 0

// Low-level pin register query macros
// -----------------------------------
#define portOutputRegister(P) ((volatile uint32_t *)(0x40001000 + (P)*0x0C))
#define portInputRegister(P)  ((volatile uint32_t *)(0x40001050 + (P)*4))
#define portModeRegister(P)	  ((volatile uint32_t *)(0x40001004 + (P)*0x0C))

// LEDs
// ----
#define PIN_LED		4u		// PA_0
#define LED_BUILTIN PIN_LED // PA_0

// Analog pins
// -----------
#define PIN_A0 1u  // PA_19
#define PIN_A1 11u // VBAT_MEAS
#define PIN_A2 0u  // dummy, this is NOT usable

static const uint8_t A0 = PIN_A0;
static const uint8_t A1 = PIN_A1;
static const uint8_t A2 = PIN_A2;

// SPI Interfaces
// --------------
#define SPI_INTERFACES_COUNT 0

// Wire Interfaces
// ---------------
#define WIRE_INTERFACES_COUNT 2
#define PIN_WIRE0_SDA		  1u  // PA_19
#define PIN_WIRE0_SCL		  0u  // PA_22
#define PIN_WIRE1_SDA		  10u // PA_23
#define PIN_WIRE1_SCL		  9u  // PA_18

// Serial ports
// ------------
// LOGUARTClass
#define PIN_SERIAL2_RX 5u // PA_29
#define PIN_SERIAL2_TX 6u // PA_30
// UARTClassOne
#define PIN_SERIAL0_RX 9u  // PA_18
#define PIN_SERIAL0_TX 10u // PA_23

#define SERIAL_PORT_USBVIRTUAL	  Serial
#define SERIAL_PORT_MONITOR		  Serial
#define SERIAL_PORT_HARDWARE	  Serial
#define SERIAL_PORT_HARDWARE_OPEN Serial

#ifdef __cplusplus
#include "LOGUARTClass.h"
extern LOGUARTClass Serial;
#endif // __cplusplus

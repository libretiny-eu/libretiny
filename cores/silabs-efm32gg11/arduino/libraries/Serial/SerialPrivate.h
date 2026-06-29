/* Copyright (c) WGM160P-LibreTiny port 2026-05-26.
 *
 * Family-private hooks for silabs-efm32gg11's SerialClass override.
 *
 * The plan (Task 26) described a standalone EfmHardwareSerial class, but the
 * common Arduino-API surface (cores/common/arduino/libraries/api/Serial/Serial.h)
 * already declares SerialClass : public HardwareSerial with a small set of
 * private virtual hooks (beginPrivate, endPrivate, configure, flush, write).
 * Families provide those overrides plus a SerialData struct for any per-port
 * state they need to carry between calls — see beken-72xx, lightning-ln882h,
 * realtek-ambz, realtek-ambz2 for the pattern.
 *
 * The RX IRQ handler lives outside the class, so SerialData also stashes a
 * pointer to the common SerialRingBuffer (assigned in beginPrivate from
 * SerialClass::rxBuf) — same trick realtek-ambz2 uses.
 */

#pragma once

#include <ArduinoPrivate.h>
#include <Serial.h>

#include "em_usart.h"

struct SerialData {
	/* USART instance handled by this Serial port. Phase 1: always USART0. */
	USART_TypeDef *uart{nullptr};
	/* USART location (LOC0..LOC31). Phase 1: USART0 LOC1 wires to WSTK VCOM. */
	uint8_t loc{0};
	/* Ring buffer pointer (== SerialClass::rxBuf) — exposed for the ISR. */
	SerialRingBuffer *buf{nullptr};
	/* RX overflow flag: set by the ISR when store_char() can't accept a byte
	 * because the ring is full. Read-and-clear via the family helper
	 * SerialClass_getOverflow() declared at the bottom of this header. */
	volatile bool rx_overflow{false};
};

/* Family extension: poll-and-clear the RX overflow flag of the default Serial
 * port. Not part of the common HardwareSerial surface — provided as a free
 * function so user code can detect ring overflow without subclassing. */
bool SerialClass_getOverflow();

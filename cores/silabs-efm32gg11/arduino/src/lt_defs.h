#pragma once

#error "Don't include this file directly"

/* Phase 1 Arduino-side toggles for silabs-efm32gg11.
 *
 * LT_ARD_HAS_SERIAL was gated to 0 until HardwareSerial.{h,cpp} landed (T26);
 * common's Arduino.h checks this macro to decide whether to pull in Serial.h.
 * Now set to 1 because cores/silabs-efm32gg11/arduino/libraries/Serial/ provides
 * the family-specific SerialClass overrides (USART0 LOC1 + RX IRQ ring buffer).
 */
#define LT_ARD_HAS_SERIAL  1
#define LT_ARD_HAS_WIFI	   1
#define LT_ARD_MD5_MBEDTLS 0

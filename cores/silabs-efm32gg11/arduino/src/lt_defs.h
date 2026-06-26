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
// MD5 via the GSDK mbedtls already compiled for the WFX Secure Link (md5.c +
// MBEDTLS_MD5_C added alongside the secure-link crypto). The Arduino Update/OTA
// path (Update.cpp) references MD5Init/Update/Final unconditionally, so this
// must be 1 for OTA to link. Independent of LT_HAS_MBEDTLS (no full TLS stack).
#define LT_ARD_MD5_MBEDTLS 1

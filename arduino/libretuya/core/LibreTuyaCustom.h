/* Copyright (c) Kuba Szczodrzyński 2022-06-06. */

#include "LibreTuyaAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set resolution of values (in bits) returned by analogRead().
 */
void analogReadResolution(int res);

/**
 * @brief Set PWM output frequency (in Hz).
 */
void analogWriteFrequency(int hz);

/**
 * @brief Set PWM output frequency (cycle period) in microseconds.
 */
void analogWritePeriod(int us);

/**
 * @brief Set resolution of values (in bits) expected by analogWrite().
 */
void analogWriteResolution(int res);

#ifdef __cplusplus
} // extern "C"
#endif

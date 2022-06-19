/* Copyright (c) Kuba Szczodrzyński 2022-06-06. */

#pragma once

#include "LibreTuyaAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Run main_task & start OS kernel (family-defined)
 */
extern bool startMainTask();

/**
 * @brief Main setup() and loop() task.
 * Not to be called directly.
 */
extern void main_task(const void *arg);

#define PIN_NONE (1 << 0)
#define PIN_GPIO (1 << 1)
#define PIN_IRQ	 (1 << 2)
#define PIN_PWM	 (1 << 3)
#define PIN_ADC	 (1 << 4)
#define PIN_DAC	 (1 << 5)
#define PIN_I2C	 (1 << 6)
#define PIN_I2S	 (1 << 7)
#define PIN_JTAG (1 << 8)
#define PIN_SPI	 (1 << 9)
#define PIN_SWD	 (1 << 10)
#define PIN_UART (1 << 11)

typedef struct {
	/**
	 * @brief GPIO name in the family SDK.
	 */
	uint32_t gpio;
	/**
	 * @brief Supported pin features.
	 */
	uint32_t features;
	/**
	 * @brief Enabled pin features. Used values are family-specific.
	 */
	uint32_t types;
	/**
	 * @brief Pin mode (direction, IRQ level, etc.).
	 */
	uint32_t mode;
} PinInfo;

extern PinInfo pinTable[];

/**
 * @brief Read voltage from analog input (in millivolts).
 */
uint16_t analogReadVoltage(pin_size_t pinNumber);

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

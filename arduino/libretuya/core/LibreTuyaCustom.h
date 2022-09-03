/* Copyright (c) Kuba Szczodrzyński 2022-06-06. */

#pragma once

#include "LibreTuyaAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Run mainTask & start OS kernel (family-defined).
 * Return false if an error occured; else do not return and
 * and keep the OS kernel running.
 */
extern bool startMainTask();

/**
 * @brief Main setup() and loop() task.
 * Not to be called directly.
 */
extern void mainTask(const void *arg);

/**
 * @brief Run periodic tasks, like printing free heap or checking millis() overflow.
 *
 * This is called during delaying operations, like yield() or delay().
 */
extern void runPeriodicTasks();

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
	 * @brief Supported pin functions.
	 */
	uint32_t supported;
	/**
	 * @brief Enabled pin functions. Used values are family-specific.
	 */
	uint32_t enabled;
	/**
	 * @brief Pin mode (direction, IRQ level, etc.).
	 */
	uint32_t mode;
} PinInfo;

extern PinInfo pinTable[];

// Custom Wiring methods

bool pinInvalid(pin_size_t pinNumber);
PinInfo *pinInfo(pin_size_t pinNumber);
bool pinSupported(PinInfo *pin, uint32_t mask);
bool pinEnabled(PinInfo *pin, uint32_t mask);
bool pinIsOutput(PinInfo *pin);
bool pinIsInput(PinInfo *pin);

int analogRead(pin_size_t pinNumber);
void analogReadResolution(int res);
void analogWriteResolution(int res);
void analogWriteFrequency(int hz);
void analogWritePeriod(int us);

extern int _analogReadResolution;
extern int _analogWriteResolution;
extern int _analogWritePeriod;

/**
 * @brief Read voltage from analog input (in millivolts).
 */
uint16_t analogReadVoltage(pin_size_t pinNumber);

/**
 * @brief Get max reading voltage for the specified pin (millivolts).
 */
uint16_t analogReadMaxVoltage(pin_size_t pinNumber);

#ifdef __cplusplus
} // extern "C"
#endif

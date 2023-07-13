/* Copyright (c) Kuba Szczodrzyński 2022-06-06. */

#pragma once

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#define PIN_MODE_ALL 0xFFFFFFFF

typedef struct PinData_s PinData;

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
	 * @brief Pin data (direction, IRQ level, etc.). The structure is family-specific.
	 */
	PinData *data;
} PinInfo;

extern PinInfo lt_arduino_pin_info_list[PINS_COUNT];
extern PinInfo *lt_arduino_pin_gpio_map[PINS_GPIO_MAX + 1];

// Custom Wiring methods

/**
 * @brief Run mainTask & start OS kernel (family-defined).
 * Return false if an error occured; else do not return and
 * and keep the OS kernel running.
 */
bool startMainTask(void);

void mainTask(const void *arg); // implemented in main.cpp
void runPeriodicTasks();		// implemented in wiring_custom.c

void pinModeRemove(pin_size_t pinNumber, uint32_t mask);
PinInfo *pinInfo(pin_size_t pinNumber);
PinInfo *pinByIndex(uint32_t index);
PinInfo *pinByGpio(uint32_t gpio);
uint32_t pinIndex(PinInfo *pin);
bool pinSupported(PinInfo *pin, uint32_t mask);
bool pinEnabled(PinInfo *pin, uint32_t mask);
void pinRemoveMode(PinInfo *pin, uint32_t mask);

/**
 * @brief Deinitialize the pin, by removing all enabled modes.
 */
inline void pinModeNone(pin_size_t pinNumber) {
	pinModeRemove(pinNumber, PIN_MODE_ALL);
}

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

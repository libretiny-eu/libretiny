/* Copyright (c) Kuba Szczodrzyński 2022-06-20. */

#include "wiring_private.h"

#if LT_HAS_FREERTOS
#include <FreeRTOS.h>
#endif

int _analogReadResolution  = 10;	// 0-1023
int _analogWriteResolution = 8;		// 0-255
int _analogWritePeriod	   = 20000; // 50 Hz

static unsigned long periodicTasks[] = {0, 0};

/**
 * @brief Run periodic tasks, like printing free heap or checking millis() overflow.
 *
 * This is called during delaying operations, like yield() or delay().
 */
void runPeriodicTasks() {
#if LT_LOG_HEAP
	if (millis() - periodicTasks[0] > 1000) {
		LT_HEAP_I();
		periodicTasks[0] = millis();
	}
#endif
#if LT_USE_TIME
	if (millis() - periodicTasks[1] > 10000) {
		gettimeofday(NULL, NULL);
		periodicTasks[1] = millis();
	}
#endif
}

/**
 * @brief Disable modes specified by 'mask'.
 */
void pinModeRemove(pin_size_t pinNumber, uint32_t mask) {
	PinInfo *pin = pinInfo(pinNumber);
	if (!pin)
		return;
	pinRemoveMode(pin, mask);
	if (pin->enabled == PIN_NONE && mask == PIN_MODE_ALL)
		pinRemoveData(pin);
}

/**
 * @brief Get PinInfo struct for the specified number.
 * Returns NULL if pin number is invalid.
 */
PinInfo *pinInfo(pin_size_t pinNumber) {
	if (pinNumber < 0 || pinNumber > PINS_GPIO_MAX)
		return NULL;
	return lt_arduino_pin_gpio_map[pinNumber];
}

/**
 * @brief Get PinInfo struct for the specified index.
 * Returns NULL if pin index is invalid.
 */
PinInfo *pinByIndex(uint32_t index) {
	if (index < 0 || index >= PINS_COUNT)
		return NULL;
	return &(lt_arduino_pin_info_list[index]);
}

/**
 * @brief Find PinInfo struct by GPIO number.
 * Returns NULL if not found.
 */
PinInfo *pinByGpio(uint32_t gpio) {
	for (uint32_t i = 0; i < PINS_COUNT; i++) {
		if (lt_arduino_pin_info_list[i].gpio == gpio)
			return &(lt_arduino_pin_info_list[i]);
	}
	return NULL;
}

/**
 * @brief Get index of PinInfo in the global pin info table.
 */
uint32_t pinIndex(PinInfo *pin) {
	return pin - lt_arduino_pin_info_list;
}

/**
 * @brief Check if pin supports all features represented by 'mask'.
 */
bool pinSupported(PinInfo *pin, uint32_t mask) {
	return (pin->supported & mask) == mask;
}

/**
 * @brief Check if pin has all features represented by 'mask' enabled.
 */
bool pinEnabled(PinInfo *pin, uint32_t mask) {
	return (pin->enabled & mask) == mask;
}

/**
 * @brief Read voltage from ADC and return a value between 0 and
 * the current reading resolution.
 */
int analogRead(pin_size_t pinNumber) {
	float voltage	 = analogReadVoltage(pinNumber);
	float maxVoltage = analogReadMaxVoltage(pinNumber);
	uint16_t ret	 = round((1 << _analogReadResolution) * voltage / maxVoltage);
	if (ret >= (1 << _analogReadResolution))
		ret = (1 << _analogReadResolution) - 1;
	return ret;
}

/**
 * @brief Set resolution of values (in bits) returned by analogRead().
 * Defaults to 10 bit (0-1023).
 */
void analogReadResolution(int res) {
	_analogReadResolution = res;
}

/**
 * @brief Set resolution of values (in bits) expected by analogWrite().
 * Defaults to 8 bit (0-255).
 */
void analogWriteResolution(int res) {
	_analogWriteResolution = res;
}

/**
 * @brief Set PWM output frequency (in Hz).
 * Defaults to 50 Hz (20,000 uS).
 */
void analogWriteFrequency(int hz) {
	_analogWritePeriod = 1E6 / hz;
}

/**
 * @brief Set PWM output frequency (cycle period) in microseconds.
 * Defaults to 20,000 uS (50 Hz).
 */
void analogWritePeriod(int us) {
	_analogWritePeriod = us;
}

__attribute__((weak)) void analogReference(uint8_t mode) {}

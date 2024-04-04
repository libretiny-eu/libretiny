/* Copyright (c) Peter Sarkozi 2023-06-17. */

#pragma once

#include <libretiny.h>

/**
 * @brief Enable GPIO Wakeup from Deep Sleep.
 *
 * @param gpio_index_map bitMap of the pins we should wake up on
 * @param on_high whether to wake up on High or Low state
 */
void lt_deep_sleep_config_gpio(uint32_t gpio_index_map, bool on_high);

/**
 * @brief Disable GPIO Wakeup on given pins
 *
 * @param gpio_index_map bitMap of the pins we should disable wake up on
 */
void lt_deep_sleep_unset_gpio(uint32_t gpio_index_map);

/**
 * @brief Configure GPIO pins to be kept floating
 *
 * This prevents pins from being configured as pullup/pulldown, based on
 * the configured wake-up mode (low/high), upon entering deep sleep.
 * This is important, as some-low power devices may be unable to ever
 * overcome the internal pull strength and change the level. One example
 * of such device is a magnetic door sensor that must be able to wake on
 * level change in both directions.
 *
 * @param gpio_index_map bitMap of the pins to set/unset as floating
 * @param enabled whether to set pins to floating or roll back to default mode
 */
void lt_deep_sleep_keep_floating_gpio(uint32_t gpio_index_map, bool enabled);

/**
 * @brief Set a sleep timer to wake up the device
 * @param sleep_duration the time in milliseconds to sleep
 */
void lt_deep_sleep_config_timer(uint32_t sleep_duration);

/**
 * @brief Start deep sleep
 */
void lt_deep_sleep_enter();

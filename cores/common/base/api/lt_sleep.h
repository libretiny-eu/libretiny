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
 * @brief Set a sleep timer to wake up the device
 * @param sleep_duration the time in milliseconds to sleep
 */
void lt_deep_sleep_config_timer(uint32_t sleep_duration);

/**
 * @brief Start deep sleep
 */
void lt_deep_sleep_enter();

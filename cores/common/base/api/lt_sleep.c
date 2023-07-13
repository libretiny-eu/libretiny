/* Copyright (c) Peter Sarkozi 2023-06-17. */

#include "lt_sleep.h"

__attribute__((weak)) void lt_deep_sleep_config_gpio(uint32_t gpio_index_map, bool on_high);

__attribute__((weak)) void lt_deep_sleep_unset_gpio(uint32_t gpio_index_map);

__attribute__((weak)) void lt_deep_sleep_config_timer(uint32_t sleep_duration);

__attribute__((weak)) void lt_deep_sleep_enter();

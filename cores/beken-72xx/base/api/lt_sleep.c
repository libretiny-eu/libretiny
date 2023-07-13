/* Copyright (c) Peter Sarkozi 2023-06-17. */

#include <libretiny.h>
#include <sdk_private.h>

static PS_DEEP_CTRL_PARAM deep_sleep_param;

void lt_deep_sleep_config_gpio(uint32_t gpio_index_map, bool on_high) {
	deep_sleep_param.wake_up_way |= PS_DEEP_WAKEUP_GPIO;
	deep_sleep_param.gpio_index_map |= gpio_index_map;
	if (on_high) {
		deep_sleep_param.gpio_edge_map &= (~gpio_index_map);
	} else {
		deep_sleep_param.gpio_edge_map |= gpio_index_map;
	}
}

void lt_deep_sleep_unset_gpio(uint32_t gpio_index_map) {
	deep_sleep_param.gpio_index_map &= (~gpio_index_map);
}

void lt_deep_sleep_config_timer(uint32_t sleep_duration) {
	deep_sleep_param.wake_up_way |= PS_DEEP_WAKEUP_RTC;
	uint64_t duration_math = 32768 * sleep_duration;
	if (duration_math / 1000 > 0xFFFFFFFF) {
		// Sleep forever
		deep_sleep_param.sleep_time = 0xFFFFFFFF;
	} else {
		deep_sleep_param.sleep_time = (duration_math / 1000) & 0xFFFFFFFF;
	}
}

void lt_deep_sleep_enter() {
	bk_misc_update_set_type(RESET_SOURCE_DEEPPS_GPIO);
	GLOBAL_INT_DECLARATION();
	GLOBAL_INT_DISABLE();
	sctrl_enter_rtos_deep_sleep((PS_DEEP_CTRL_PARAM *)&deep_sleep_param);
	ps_delay(500);
	GLOBAL_INT_RESTORE();
}

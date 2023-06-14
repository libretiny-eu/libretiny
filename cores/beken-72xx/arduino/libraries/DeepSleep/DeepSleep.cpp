/* Copyright (c) Peter Sarkozi 2023-06-13. */

#include <manual_ps.h>
#include <manual_ps_pub.h>
#include <DeepSleep.h>
#include "DeepSleepPrivate.h"

  void bk_enter_deep_sleep_manual(uint32_t g_gpio_index_map, uint32_t g_gpio_edge_map)
  {
	PS_DEEP_CTRL_PARAM deep_sleep_param;

	deep_sleep_param.wake_up_way = PS_DEEP_WAKEUP_GPIO;
        deep_sleep_param.gpio_index_map = g_gpio_index_map;
	deep_sleep_param.gpio_edge_map = g_gpio_edge_map;
        extern void bk_enter_deep_sleep_mode(ps_deep_ctrl*);
	bk_enter_deep_sleep_mode((PS_DEEP_CTRL_PARAM*)&deep_sleep_param);
  }



LibreTinyDeepSleep::LibreTinyDeepSleep() {
        data = (LibreTinyDeepSleep *)calloc(1, sizeof(LibreTinyDeepSleep));
}

LibreTinyDeepSleep::~LibreTinyDeepSleep() {
        free(data);
        data = NULL;
}
void LibreTinyDeepSleep::enableTimerWakeup(uint32_t duration) {
  extern void deep_sleep_wakeup_with_timer(uint32_t sleep_time);
  deep_sleep_wakeup_with_timer(duration);
}
void LibreTinyDeepSleep::enableGpioWakeup(uint32_t gpioIndexMap, bool onHigh) {
  DATA->gpioMap = gpioIndexMap;
  DATA->onHigh = onHigh;
}
void LibreTinyDeepSleep::enter() {
#if CFG_SOC_NAME == SOC_BK7231U
  extern void deep_sleep_wakeup_with_gpio(uint32_t gpio_index_map, uint32_t gpio_edge_map);
  deep_sleep_wakeup_with_gpio(DATA->gpioMap, DATA->onHigh ? 0 : 0xFFFFFFFF);
#endif
  bk_enter_deep_sleep_manual(DATA->gpioMap, DATA->onHigh ? 0 : 0xFFFFFFFF);
}

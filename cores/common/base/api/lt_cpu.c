/* Copyright (c) Kuba Szczodrzyński 2022-04-29. */

#include "lt_cpu.h"

#if LT_HAS_FREERTOS
#include <FreeRTOS.h>
#include <task.h>
#endif

lt_cpu_family_t lt_cpu_get_family() {
	return FAMILY;
}

const char *lt_cpu_get_family_name() {
	return STRINGIFY_MACRO(FAMILY) + 2;
}

__attribute__((weak)) lt_cpu_model_t lt_cpu_get_model() {
	return MCU;
}

const char *lt_cpu_get_model_name() {
	return STRINGIFY_MACRO(MCU);
}

const char *lt_cpu_get_model_code() {
	return STRINGIFY_MACRO(MCULC);
}

__attribute__((weak)) uint32_t lt_cpu_get_unique_id() {
	return lt_cpu_get_mac_id();
}

__attribute__((weak)) uint32_t lt_cpu_get_mac_id() {
	uint8_t mac[6];
	lt_get_device_mac(mac);
	return (mac[3] << 0) | (mac[4] << 8) | (mac[5] << 16);
}

__attribute__((weak)) uint8_t lt_cpu_get_core_count() {
	return 1;
}

#if LT_HAS_FREERTOS
__attribute__((weak)) uint32_t lt_cpu_get_freq() {
	return configCPU_CLOCK_HZ;
}
#endif

uint32_t lt_cpu_get_freq_mhz() {
	return lt_cpu_get_freq() / 1000000;
}

#if LT_HAS_FREERTOS
__attribute__((weak)) uint32_t lt_cpu_get_cycle_count() {
	return xTaskGetTickCount() * (configCPU_CLOCK_HZ / configTICK_RATE_HZ);
}
#endif

/* Copyright (c) Kuba Szczodrzyński 2022-04-29. */

#include "lt_api.h"

#include <fal.h>

#if LT_HAS_FREERTOS
#include <FreeRTOS.h>
#include <task.h>
#endif

/* _____ _____  _    _
  / ____|  __ \| |  | |
 | |    | |__) | |  | |
 | |    |  ___/| |  | |
 | |____| |    | |__| |
  \_____|_|     \____*/
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

/*_____             _
 |  __ \           (_)
 | |  | | _____   ___  ___ ___
 | |  | |/ _ \ \ / / |/ __/ _ \
 | |__| |  __/\ V /| | (_|  __/
 |_____/ \___| \_/ |_|\___\__*/
static char *device_name = NULL;

const char *lt_get_version() {
	return LT_VERSION_STR;
}

const char *lt_get_board_code() {
	return LT_BOARD_STR;
}

const char *lt_get_device_name() {
	if (device_name)
		return device_name;
	uint32_t chip_id = lt_cpu_get_mac_id();
	uint8_t *id		 = (uint8_t *)&chip_id;

	const char *model = lt_cpu_get_model_code();
	uint8_t model_len = strlen(model);
	device_name		  = (char *)malloc(3 + model_len + 1 + 6 + 1);

	sprintf(device_name, "LT-%s-%02x%02x%02x", model, id[0], id[1], id[2]);
	return device_name;
}

__attribute__((weak)) bool lt_reboot_wdt() {
	if (!lt_wdt_enable(1L))
		return false;
	while (1) {}
}

__attribute__((weak)) bool lt_reboot_download_mode() {
	return false;
}

const char *lt_get_reboot_reason_name(lt_reboot_reason_t reason) {
	if (!reason)
		reason = lt_get_reboot_reason();
	switch (reason) {
		case RESET_REASON_POWER:
			return "Power-On";
		case RESET_REASON_BROWNOUT:
			return "Brownout";
		case RESET_REASON_HARDWARE:
			return "HW Reboot";
		case RESET_REASON_SOFTWARE:
			return "SW Reboot";
		case RESET_REASON_WATCHDOG:
			return "WDT Reset";
		case RESET_REASON_CRASH:
			return "Crash";
		case RESET_REASON_SLEEP:
			return "Sleep Wakeup";
		default:
			return "Unknown";
	}
}

__attribute__((weak)) void lt_gpio_recover() {
	// nop by default
}

/*______ _           _
 |  ____| |         | |
 | |__  | | __ _ ___| |__
 |  __| | |/ _` / __| '_ \
 | |    | | (_| \__ \ | | |
 |_|    |_|\__,_|___/_| |*/
__attribute__((weak)) uint32_t lt_flash_get_size() {
	lt_flash_id_t id = lt_flash_get_id();
	if (id.chip_size_id >= 0x14 && id.chip_size_id <= 0x19) {
		return (1 << id.chip_size_id);
	}
#ifdef FLASH_LENGTH
	return FLASH_LENGTH;
#else
	return 0;
#endif
}

bool lt_flash_erase(uint32_t offset, size_t length) {
	return fal_partition_erase(fal_root_part, offset, length) >= 0;
}

bool lt_flash_erase_block(uint32_t offset) {
	return fal_partition_erase(fal_root_part, offset, 1) >= 0;
}

uint32_t lt_flash_read(uint32_t offset, uint8_t *data, size_t length) {
	int ret = fal_partition_read(fal_root_part, offset, data, length);
	if (ret == -1)
		return 0;
	return ret;
}

uint32_t lt_flash_write(uint32_t offset, const uint8_t *data, size_t length) {
	int ret = fal_partition_write(fal_root_part, offset, data, length);
	if (ret == -1)
		return 0;
	return ret;
}

/*__  __
 |  \/  |
 | \  / | ___ _ __ ___   ___  _ __ _   _
 | |\/| |/ _ \ '_ ` _ \ / _ \| '__| | | |
 | |  | |  __/ | | | | | (_) | |  | |_| |
 |_|  |_|\___|_| |_| |_|\___/|_|   \__, |
									__/ |
								   |__*/
#if LT_HAS_FREERTOS
__attribute__((weak)) uint32_t lt_heap_get_size() {
	return configTOTAL_HEAP_SIZE;
}

__attribute__((weak)) uint32_t lt_heap_get_free() {
	return xPortGetFreeHeapSize();
}

__attribute__((weak)) uint32_t lt_heap_get_min_free() {
	return xPortGetMinimumEverFreeHeapSize();
}
#endif

__attribute__((weak)) uint32_t lt_heap_get_max_alloc() {
	return 0;
}

/* ____ _______
  / __ \__   __|/\
 | |  | | | |  /  \
 | |  | | | | / /\ \
 | |__| | | |/ ____ \
  \____/  |_/_/    \*/
bool lt_ota_can_rollback() {
	if (lt_ota_get_type() != OTA_TYPE_DUAL)
		return false;
	uint8_t current = lt_ota_dual_get_current();
	return lt_ota_is_valid(current ^ 0b11);
}

uf2_ota_scheme_t lt_ota_get_uf2_scheme() {
	if (lt_ota_get_type() == OTA_TYPE_SINGLE)
		return UF2_SCHEME_DEVICE_SINGLE;
	uint8_t current = lt_ota_dual_get_current();
	// UF2_SCHEME_DEVICE_DUAL_1 or UF2_SCHEME_DEVICE_DUAL_2
	return (uf2_ota_scheme_t)(current ^ 0b11);
}

/*_    _ _   _ _
 | |  | | | (_) |
 | |  | | |_ _| |___
 | |  | | __| | / __|
 | |__| | |_| | \__ \
  \____/ \__|_|_|__*/
void lt_rand_bytes(uint8_t *buf, size_t len) {
	int *data = (int *)buf;
	size_t i;
	for (i = 0; len >= sizeof(int); len -= sizeof(int)) {
		data[i++] = rand();
	}
	if (len) {
		int rem				= rand();
		unsigned char *pRem = (unsigned char *)&rem;
		memcpy(buf + i * sizeof(int), pRem, len);
	}
}

void hexdump(const uint8_t *buf, size_t len, uint32_t offset, uint8_t width) {
	uint16_t pos = 0;
	while (pos < len) {
		// print hex offset
		printf("%06lx ", offset + pos);
		// calculate current line width
		uint8_t lineWidth = MIN(width, len - pos);
		// print hexadecimal representation
		for (uint8_t i = 0; i < lineWidth; i++) {
			if (i % 8 == 0) {
				printf(" ");
			}
			printf("%02x ", buf[pos + i]);
		}
		// print ascii representation
		printf(" |");
		for (uint8_t i = 0; i < lineWidth; i++) {
			char c = buf[pos + i];
			putchar((c >= 0x20 && c <= 0x7f) ? c : '.');
		}
		puts("|\r");
		pos += lineWidth;
	}
}

/*_          __   _       _         _
 \ \        / /  | |     | |       | |
  \ \  /\  / /_ _| |_ ___| |__   __| | ___   __ _
   \ \/  \/ / _` | __/ __| '_ \ / _` |/ _ \ / _` |
	\  /\  / (_| | || (__| | | | (_| | (_) | (_| |
	 \/  \/ \__,_|\__\___|_| |_|\__,_|\___/ \__, |
											 __/ |
											|___*/
__attribute__((weak)) bool lt_wdt_enable(uint32_t timeout) {
	return false;
}

__attribute__((weak)) void lt_wdt_disable() {}

__attribute__((weak)) void lt_wdt_feed() {}


/**
* Deep Sleep
**/

__attribute__((weak)) void lt_deep_sleep_config_gpio(uint32_t gpio_index_map, bool on_high);

__attribute__((weak)) void lt_deep_sleep_config_timer(uint32_t sleep_duration);

__attribute__((weak)) void lt_deep_sleep_enter();

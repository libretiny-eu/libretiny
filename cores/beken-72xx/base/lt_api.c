/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#include <libretiny.h>
#include <sdk_private.h>

// can't include <flash.h> as it collides with <Flash.h> on Windows -_-
#define REG_FLASH_BASE		 0x00803000
#define REG_FLASH_OPERATE_SW (REG_FLASH_BASE + 0 * 4)
#define REG_FLASH_RDID		 (REG_FLASH_BASE + 4 * 4)
#define FLASH_BUSY_SW		 (0x01UL << 31)
#define FLASH_WP_VALUE		 (0x01UL << 30)
#define FLASH_OP_SW			 (0x01UL << 29)
#define FLASH_OP_TYPE_POS	 24
#define FLASH_OP_RDID		 20

void lt_init_family() {
	// set default UART output port
	uart_print_port = LT_UART_DEFAULT_PORT - 1;
}

/* _____ _____  _    _
  / ____|  __ \| |  | |
 | |    | |__) | |  | |
 | |    |  ___/| |  | |
 | |____| |    | |__| |
  \_____|_|     \____*/
lt_cpu_model_t lt_cpu_get_model() {
	uint8_t chipId = *(uint8_t *)(SCTRL_CHIP_ID);
	return CPU_MODEL_ENUM(FAMILY, chipId);
}

uint32_t lt_cpu_get_unique_id() {
	return lt_cpu_get_mac_id();
}

uint32_t lt_cpu_get_mac_id() {
	uint8_t mac[6];
	cfg_load_mac(mac); // force loading MAC from TLV (ignore user-set WiFi MAC)
	return (mac[3]) | (mac[4] << 8) | (mac[5] << 16);
}

const char *lt_cpu_get_core_type() {
	return "ARM968E-S";
}

/*_____             _
 |  __ \           (_)
 | |  | | _____   ___  ___ ___
 | |  | |/ _ \ \ / / |/ __/ _ \
 | |__| |  __/\ V /| | (_|  __/
 |_____/ \___| \_/ |_|\___\__*/
void lt_reboot() {
	bk_reboot();
}

bool lt_reboot_download_mode() {
	bk_reboot();
	return true;
}

lt_reboot_reason_t lt_get_reboot_reason() {
	switch (bk_misc_get_start_type()) {
		case RESET_SOURCE_POWERON:
			return REBOOT_REASON_POWER;
		case RESET_SOURCE_REBOOT:
			return REBOOT_REASON_SOFTWARE;
		case RESET_SOURCE_WATCHDOG:
			return REBOOT_REASON_WATCHDOG;
		case RESET_SOURCE_CRASH_XAT0:
		case RESET_SOURCE_CRASH_UNDEFINED:
		case RESET_SOURCE_CRASH_PREFETCH_ABORT:
		case RESET_SOURCE_CRASH_DATA_ABORT:
		case RESET_SOURCE_CRASH_UNUSED:
		case RESET_SOURCE_CRASH_PER_XAT0:
			return REBOOT_REASON_CRASH;
		case RESET_SOURCE_DEEPPS_GPIO:
		case RESET_SOURCE_DEEPPS_RTC:
		case RESET_SOURCE_DEEPPS_USB:
			return REBOOT_REASON_SLEEP;
		default:
			return REBOOT_REASON_UNKNOWN;
	}
}

/*______ _           _
 |  ____| |         | |
 | |__  | | __ _ ___| |__
 |  __| | |/ _` / __| '_ \
 | |    | | (_| \__ \ | | |
 |_|    |_|\__,_|___/_| |*/
lt_flash_id_t lt_flash_get_id() {
	uint32_t data = (FLASH_OP_RDID << FLASH_OP_TYPE_POS) | FLASH_OP_SW | FLASH_WP_VALUE;
	REG_WRITE(REG_FLASH_OPERATE_SW, data);
	while (REG_READ(REG_FLASH_OPERATE_SW) & FLASH_BUSY_SW) {}
	lt_flash_id_t id = {
		.manufacturer_id = REG_RD8(REG_FLASH_RDID, 2),
		.chip_id		 = REG_RD8(REG_FLASH_RDID, 1),
		.chip_size_id	 = REG_RD8(REG_FLASH_RDID, 0),
	};
	return id;
}

/*__  __
 |  \/  |
 | \  / | ___ _ __ ___   ___  _ __ _   _
 | |\/| |/ _ \ '_ ` _ \ / _ \| '__| | | |
 | |  | |  __/ | | | | | (_) | |  | |_| |
 |_|  |_|\___|_| |_| |_|\___/|_|   \__, |
									__/ |
								   |__*/
uint32_t lt_ram_get_size() {
	return 256 * 1024;
}

uint32_t lt_heap_get_size() {
#if configDYNAMIC_HEAP_SIZE
	extern unsigned char _empty_ram;
#if CFG_SOC_NAME == SOC_BK7231N
	return (0x00400000 + 192 * 1024) - (uint32_t)(&_empty_ram);
#else
	return (0x00400000 + 256 * 1024) - (uint32_t)(&_empty_ram);
#endif
#else
	return configTOTAL_HEAP_SIZE;
#endif
}

/* ____ _______
  / __ \__   __|/\
 | |  | | | |  /  \
 | |  | | | | / /\ \
 | |__| | | |/ ____ \
  \____/  |_/_/    \*/

lt_ota_type_t lt_ota_get_type() {
	return OTA_TYPE_SINGLE;
}

bool lt_ota_is_valid(uint8_t index) {
	if (index != 0)
		return false;
	// check download RBL
	// TODO: maybe check header CRC or even binary hashes
	uint32_t magic;
	lt_flash_read(FLASH_DOWNLOAD_OFFSET, (uint8_t *)&magic, 4);
	return magic == 0x004C4252; // "RBL\0", little-endian
}

uint8_t lt_ota_dual_get_current() {
	return 0;
}

uint8_t lt_ota_dual_get_stored() {
	return 0;
}

bool lt_ota_switch(bool revert) {
	if (!lt_ota_is_valid(0))
		// no valid "download" image
		// - return false when trying to activate
		// - return true when trying to revert
		return revert;
	if (revert) {
		// there's a valid "download" image, which has to be removed
		return lt_flash_erase_block(FLASH_DOWNLOAD_OFFSET);
	}
	return true;
}

/*_          __   _       _         _
 \ \        / /  | |     | |       | |
  \ \  /\  / /_ _| |_ ___| |__   __| | ___   __ _
   \ \/  \/ / _` | __/ __| '_ \ / _` |/ _ \ / _` |
	\  /\  / (_| | || (__| | | | (_| | (_) | (_| |
	 \/  \/ \__,_|\__\___|_| |_|\__,_|\___/ \__, |
											 __/ |
											|___*/
bool lt_wdt_enable(uint32_t timeout) {
	wdt_ctrl(WCMD_SET_PERIOD, &timeout);
	wdt_ctrl(WCMD_POWER_UP, NULL);
	return true;
}

void lt_wdt_disable() {
	wdt_ctrl(WCMD_POWER_DOWN, NULL);
}

void lt_wdt_feed() {
	wdt_ctrl(WCMD_RELOAD_PERIOD, NULL);
}

/**
 * Deep Sleep
 **/

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

void lt_deep_sleep_config_timer(uint32_t sleep_duration) {
	deep_sleep_param.wake_up_way |= PS_DEEP_WAKEUP_RTC;
	uint64_t duration_math = 32768*sleep_duration;
	if (duration_math > 0xFFFFFFFF) {
		// Sleep forever
		deep_sleep_param.sleep_time = 0xFFFFFFFF;
	} else {
		deep_sleep_param.sleep_time = (duration_math/1000) & 0xFFFFFFFF;
	}
}

void lt_deep_sleep_enter() {
	extern void sctrl_enter_rtos_deep_sleep(PS_DEEP_CTRL_PARAM *deep_param);
	extern void ps_delay(volatile UINT16 times);
	bk_misc_update_set_type(RESET_SOURCE_DEEPPS_GPIO);
	GLOBAL_INT_DECLARATION();
	GLOBAL_INT_DISABLE();
	sctrl_enter_rtos_deep_sleep((PS_DEEP_CTRL_PARAM *)&deep_sleep_param);
	ps_delay ( 500 );
	GLOBAL_INT_RESTORE();
}

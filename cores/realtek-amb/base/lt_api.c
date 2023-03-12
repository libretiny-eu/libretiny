/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#include <libretuya.h>
#include <sdk_private.h>

extern uint32_t GlobalDebugEnable;
extern uint16_t GlobalDebugLevel;
extern uint8_t GlobalPrivateLog;
extern uint8_t lt_uart_port;

void lt_init_family() {
	// make the SDK less verbose by default
	GlobalDebugEnable = 0;
	GlobalPrivateLog  = 0;
	lt_uart_port	  = LT_UART_DEFAULT_PORT;
}

/* _____ _____  _    _
  / ____|  __ \| |  | |
 | |    | |__) | |  | |
 | |    |  ___/| |  | |
 | |____| |    | |__| |
  \_____|_|     \____*/
lt_cpu_model_t lt_cpu_get_model() {
	uint8_t chipId;
	EFUSE_OneByteReadROM(9902, 0xF8, &chipId, L25EOUTVOLTAGE);
	return CPU_MODEL_ENUM(FAMILY, chipId);
}

uint32_t lt_cpu_get_unique_id() {
	return lt_cpu_get_mac_id();
}

uint32_t lt_cpu_get_mac_id() {
	uint32_t chipId = 0;
	uint8_t *id		= (uint8_t *)&chipId;
	// 9902 was extracted from ROM disassembly, probably not needed
	/* EFUSE_OneByteReadROM(9902, 0x3B, id + 0, L25EOUTVOLTAGE);
	EFUSE_OneByteReadROM(9902, 0x3C, id + 1, L25EOUTVOLTAGE);
	EFUSE_OneByteReadROM(9902, 0x3D, id + 2, L25EOUTVOLTAGE); */
	// new method, based on EFUSE logical map
	uint8_t *efuse = (uint8_t *)malloc(512);
	// TODO do what EFUSE_LogicalMapRead() does, and read only the used data
	EFUSE_LogicalMap_Read(efuse);
	memcpy(id, efuse + 0x11A + 3, 3);
	free(efuse);
	return chipId;
}

const char *lt_cpu_get_core_type() {
	return "ARM Cortex-M4F";
}

uint32_t lt_cpu_get_freq() {
	return CPU_ClkGet(false);
}

/*_____             _
 |  __ \           (_)
 | |  | | _____   ___  ___ ___
 | |  | |/ _ \ \ / / |/ __/ _ \
 | |__| |  __/\ V /| | (_|  __/
 |_____/ \___| \_/ |_|\___\__*/
void lt_reboot() {
	// The Watchdog Way
	lt_wdt_enable(1L);
	while (1) {}
}

bool lt_reboot_download_mode() {
	// mww 0x40000138 0x8
	HAL_WRITE32(SYSTEM_CTRL_BASE, REG_SYS_NORESET_FF, 0x08);
	// reboot it the ugly way
	sys_reset();
	while (1) {}
	return true;
}

lt_reboot_reason_t lt_get_reboot_reason() {
	// TODO
	return REBOOT_REASON_UNKNOWN;
}

void lt_gpio_recover() {
	// PA14 and PA15 are apparently unusable with SWD enabled
	sys_jtag_off();
	Pinmux_Config(PA_14, PINMUX_FUNCTION_GPIO);
	Pinmux_Config(PA_15, PINMUX_FUNCTION_GPIO);
}

/*______ _           _
 |  ____| |         | |
 | |__  | | __ _ ___| |__
 |  __| | |/ _` / __| '_ \
 | |    | | (_| \__ \ | | |
 |_|    |_|\__,_|___/_| |*/
lt_flash_id_t lt_flash_get_id() {
	lt_flash_id_t id;
	uint8_t idBytes[3];
	flash_read_id(NULL, idBytes, 3);
	id.manufacturer_id = idBytes[0];
	id.chip_id		   = idBytes[1];
	id.chip_size_id	   = idBytes[2];
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

/* ____ _______
  / __ \__   __|/\
 | |  | | | |  /  \
 | |  | | | | / /\ \
 | |__| | | |/ ____ \
  \____/  |_/_/    \*/
lt_ota_type_t lt_ota_get_type() {
	return OTA_TYPE_DUAL;
}

bool lt_ota_is_valid(uint8_t index) {
	uint32_t offset;
	switch (index) {
		case 1:
			offset = FLASH_OTA1_OFFSET;
			break;
		case 2:
			offset = FLASH_OTA2_OFFSET;
			break;
		default:
			return false;
	}
	uint8_t *address = (uint8_t *)(SPI_FLASH_BASE + offset);
	return memcmp(address, "81958711", 8) == 0;
}

uint8_t lt_ota_dual_get_current() {
	// RTL8710B is XIP, so check the code offset in flash
	uint32_t addr = (uint32_t)lt_log;
	uint32_t offs = addr - SPI_FLASH_BASE;
	return offs > FLASH_OTA2_OFFSET ? 2 : 1;
}

uint8_t lt_ota_dual_get_stored() {
	uint32_t *ota_address = (uint32_t *)0x8009000;
	if (*ota_address == 0xFFFFFFFF)
		return 1;
	uint32_t ota_counter = *((uint32_t *)0x8009004);
	// even count of zero-bits means OTA1, odd count means OTA2
	// this allows to switch OTA images by simply clearing next bits,
	// without needing to erase the flash
	uint8_t count = 0;
	for (uint8_t i = 0; i < 32; i++) {
		if ((ota_counter & (1 << i)) == 0)
			count++;
	}
	return 1 + (count % 2);
}

bool lt_ota_switch(bool revert) {
	uint8_t current = lt_ota_dual_get_current();
	uint8_t stored	= lt_ota_dual_get_stored();
	if ((current == stored) == revert)
		return true;

	if (!lt_ota_is_valid(stored ^ 0b11))
		return false;

	// - read current OTA switch value from 0x9004
	// - reset OTA switch to 0xFFFFFFFE if it's 0x0
	// - else check first non-zero bit of OTA switch
	// - write OTA switch with first non-zero bit cleared

	uint32_t value = HAL_READ32(SPI_FLASH_BASE, FLASH_SYSTEM_OFFSET + 4);
	if (value == 0) {
		uint8_t *system = (uint8_t *)malloc(64);
		lt_flash_read(FLASH_SYSTEM_OFFSET, system, 64);
		// reset OTA switch
		((uint32_t *)system)[1] = -2;
		lt_flash_erase_block(FLASH_SYSTEM_OFFSET);
		return lt_flash_write(FLASH_SYSTEM_OFFSET, system, 64);
	}

	// clear first non-zero bit
	value <<= 1;
	// write OTA switch to flash
	flash_write_word(NULL, FLASH_SYSTEM_OFFSET + 4, value);
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
	watchdog_init(timeout);
	watchdog_start();
	return true;
}

void lt_wdt_disable() {
	watchdog_stop();
}

void lt_wdt_feed() {
	watchdog_refresh();
}

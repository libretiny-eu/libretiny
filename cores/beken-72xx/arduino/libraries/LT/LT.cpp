/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <LT.h>

// can't include <flash.h> as it collides with <Flash.h> on Windows -_-
#include <Flash/Flash.h>

#define REG_FLASH_BASE		 0x00803000
#define REG_FLASH_OPERATE_SW (REG_FLASH_BASE + 0 * 4)
#define REG_FLASH_RDID		 (REG_FLASH_BASE + 4 * 4)
#define FLASH_BUSY_SW		 (0x01UL << 31)
#define FLASH_WP_VALUE		 (0x01UL << 30)
#define FLASH_OP_SW			 (0x01UL << 29)
#define FLASH_OP_TYPE_POS	 24
#define FLASH_OP_RDID		 20

extern "C" {

#include <include.h>

#include <flash_pub.h>
#include <param_config.h>
#include <start_type_pub.h>
#include <sys_ctrl.h>
#include <sys_rtos.h>
#include <wdt_pub.h>
#include <wlan_ui_pub.h>

extern uint8_t system_mac[];
extern uint32_t wdt_ctrl(uint32_t cmd, void *param);

} // extern "C"

void LibreTuya::restart() {
	bk_reboot();
}

void LibreTuya::restartDownloadMode() {
	bk_reboot();
}

/* CPU-related */

ChipType LibreTuya::getChipType() {
	uint8_t chipId = *(uint8_t *)(SCTRL_CHIP_ID);
	return CHIP_TYPE_ENUM(FAMILY, chipId);
}

const char *LibreTuya::getChipModel() {
	return STRINGIFY_MACRO(MCU);
}

uint32_t LibreTuya::getChipId() {
	uint8_t mac[6];
	cfg_load_mac(mac); // force loading MAC from TLV (ignore user-set WiFi MAC)
	return (mac[3]) | (mac[4] << 8) | (mac[5] << 16);
}

uint8_t LibreTuya::getChipCores() {
	return 1;
}

const char *LibreTuya::getChipCoreType() {
	return "ARM968E-S";
}

uint32_t LibreTuya::getCpuFreq() {
	return configCPU_CLOCK_HZ;
}

uint32_t LibreTuya::getCycleCount() {
	// TODO
	return 0;
}

/* Flash memory utilities */

FlashId LibreTuya::getFlashChipId() {
	uint32_t data = (FLASH_OP_RDID << FLASH_OP_TYPE_POS) | FLASH_OP_SW | FLASH_WP_VALUE;
	REG_WRITE(REG_FLASH_OPERATE_SW, data);
	while (REG_READ(REG_FLASH_OPERATE_SW) & FLASH_BUSY_SW) {}
	FlashId id = {
		.manufacturerId = REG_RD8(REG_FLASH_RDID, 2),
		.chipId			= REG_RD8(REG_FLASH_RDID, 1),
		.chipSizeId		= REG_RD8(REG_FLASH_RDID, 0),
	};
	return id;
}

/* Memory management */

uint32_t LibreTuya::getRamSize() {
	return 256 * 1024;
}

uint32_t LibreTuya::getHeapSize() {
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

uint32_t LibreTuya::getFreeHeap() {
	return xPortGetFreeHeapSize();
}

uint32_t LibreTuya::getMinFreeHeap() {
	return xPortGetMinimumEverFreeHeapSize();
}

uint32_t LibreTuya::getMaxAllocHeap() {
	return 0;
}

/* OTA-related */

static int8_t otaImage2Valid = -1;

uint8_t LibreTuya::otaGetRunning() {
	// Beken has bootloader-based OTA, running app is always index 1
	return 1;
}

uint8_t LibreTuya::otaGetStoredIndex() {
	return otaHasImage2() ? 2 : 1;
}

bool LibreTuya::otaSupportsDual() {
	return true;
}

bool LibreTuya::otaHasImage1() {
	return true;
}

bool LibreTuya::otaHasImage2() {
	if (otaImage2Valid != -1)
		return otaImage2Valid;
	// check download RBL
	// TODO: maybe check header CRC or even binary hashes
	uint32_t magic;
	Flash.readBlock(FLASH_DOWNLOAD_OFFSET, (uint8_t *)&magic, 4);
	otaImage2Valid = magic == 0x004C4252; // "RBL\0", little-endian
	return otaImage2Valid;
}

bool LibreTuya::otaSwitch(bool force) {
	// no need to check otaGetStoredIndex() as it does the same as otaHasImage2()

	// force checking validity again
	otaImage2Valid = -1;

	if (otaHasImage2() && force) {
		// "rollback" - abort bootloader upgrade operation by wiping first sector
		return Flash.eraseSector(FLASH_DOWNLOAD_OFFSET);
	}

	return otaHasImage2(); // false if second image is not valid
}

/* Watchdog */

bool LibreTuya::wdtEnable(uint32_t timeout) {
	wdt_ctrl(WCMD_SET_PERIOD, &timeout);
	wdt_ctrl(WCMD_POWER_UP, NULL);
}

void LibreTuya::wdtDisable() {
	wdt_ctrl(WCMD_POWER_DOWN, NULL);
}

void LibreTuya::wdtFeed() {
	wdt_ctrl(WCMD_RELOAD_PERIOD, NULL);
}

/* Global instance */

LibreTuya LT;
LibreTuya ESP = LT;

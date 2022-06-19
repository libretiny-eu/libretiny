/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <LibreTuyaAPI.h>

// can't include <flash.h> as it collides with <Flash.h> on Windows -_-
#define REG_FLASH_BASE		 0x00803000
#define REG_FLASH_OPERATE_SW (REG_FLASH_BASE + 0 * 4)
#define REG_FLASH_RDID		 (REG_FLASH_BASE + 4 * 4)
#define FLASH_BUSY_SW		 (0x01UL << 31)
#define FLASH_WP_VALUE		 (0x01UL << 30)
#define FLASH_OP_SW			 (0x01UL << 29)
#define FLASH_OP_TYPE_POS	 24
#define FLASH_OP_RDID		 20

extern "C" {
#include <flash_pub.h>
#include <sys_ctrl.h>
#include <sys_rtos.h>
}

void LibreTuya::restart() {
	// TODO
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
	// TODO determine if this really is unique
	uint32_t chipId = REG_READ(SCTRL_DEVICE_ID);
	chipId &= 0xFFFFFF;
	return chipId;
}

uint8_t LibreTuya::getChipCores() {
	return 1;
}

const char *LibreTuya::getChipCoreType() {
	return "ARM968E-S";
}

uint32_t LibreTuya::getCpuFreq() {
	// TODO
	return 0;
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
	return configTOTAL_HEAP_SIZE;
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

uint8_t LibreTuya::otaGetStoredIndex() {
	return 1;
}

bool LibreTuya::otaSupportsDual() {
	return false;
}

bool LibreTuya::otaHasImage1() {
	return true;
}

bool LibreTuya::otaHasImage2() {
	return false;
}

bool LibreTuya::otaSwitch(bool force) {
	return true;
}

/* Global instance */

LibreTuya LT;

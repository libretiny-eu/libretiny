/* Copyright (c) Kuba Szczodrzyński 2022-05-28. */

#include <LibreTuyaAPI.h>

#include <Flash.h>

extern "C" {
#include <flash_api.h>
#include <rtl8710b.h>
#include <sys_api.h>
#include <wdt_api.h>
}

void LibreTuya::restart() {
	// The Watchdog Way
	wdtEnable(1L);
	while (1) {}
}

void LibreTuya::restartDownloadMode() {
	// mww 0x40000138 0x8
	HAL_WRITE32(SYSTEM_CTRL_BASE, REG_SYS_NORESET_FF, 0x08);
	// reboot it the ugly way
	sys_reset();
	while (1) {}
}

void LibreTuya::gpioRecover() {
	// PA14 and PA15 are apparently unusable with SWD enabled
	sys_jtag_off();
	Pinmux_Config(PA_14, PINMUX_FUNCTION_GPIO);
	Pinmux_Config(PA_15, PINMUX_FUNCTION_GPIO);
}

/* CPU-related */

ChipType LibreTuya::getChipType() {
	uint8_t chipId;
	EFUSE_OneByteReadROM(9902, 0xF8, &chipId, L25EOUTVOLTAGE);
	return CHIP_TYPE_ENUM(FAMILY, chipId);
}

const char *LibreTuya::getChipModel() {
	return STRINGIFY_MACRO(MCU);
}

uint32_t LibreTuya::getChipId() {
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

uint8_t LibreTuya::getChipCores() {
	return 1;
}

const char *LibreTuya::getChipCoreType() {
	return "ARM Cortex-M4F";
}

uint32_t LibreTuya::getCpuFreq() {
	return CPU_ClkGet(false);
}

uint32_t LibreTuya::getCycleCount() {
	return microsecondsToClockCycles(micros());
}

/* Flash memory utilities */

FlashId LibreTuya::getFlashChipId() {
	FlashId id;
	uint8_t idBytes[3];
	flash_read_id(NULL, idBytes, 3);
	id.manufacturerId = idBytes[0];
	id.chipId		  = idBytes[1];
	id.chipSizeId	  = idBytes[2];
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

uint8_t LibreTuya::otaGetRunning() {
	// RTL8710B is XIP, so check the code offset in flash
	uint32_t addr = (uint32_t)lt_log;
	uint32_t offs = addr - SPI_FLASH_BASE;
	return offs > FLASH_OTA2_OFFSET ? 2 : 1;
}

uint8_t LibreTuya::otaGetStoredIndex() {
	uint32_t *otaAddress = (uint32_t *)0x8009000;
	if (*otaAddress == 0xFFFFFFFF)
		return 1;
	uint32_t otaCounter = *((uint32_t *)0x8009004);
	// even count of zero-bits means OTA1, odd count means OTA2
	// this allows to switch OTA images by simply clearing next bits,
	// without needing to erase the flash
	uint8_t count = 0;
	for (uint8_t i = 0; i < 32; i++) {
		if ((otaCounter & (1 << i)) == 0)
			count++;
	}
	return 1 + (count % 2);
}

bool LibreTuya::otaSupportsDual() {
	return true;
}

bool LibreTuya::otaHasImage1() {
	uint8_t *ota1Addr = (uint8_t *)(SPI_FLASH_BASE + FLASH_OTA1_OFFSET);
	return memcmp(ota1Addr, "81958711", 8) == 0;
}

bool LibreTuya::otaHasImage2() {
	uint8_t *ota2Addr = (uint8_t *)(SPI_FLASH_BASE + FLASH_OTA2_OFFSET);
	return memcmp(ota2Addr, "81958711", 8) == 0;
}

bool LibreTuya::otaSwitch(bool force) {
	if (!force && otaGetRunning() != otaGetStoredIndex())
		// OTA has already been switched
		return true;
	// - read current OTA switch value from 0x9004
	// - reset OTA switch to 0xFFFFFFFE if it's 0x0
	// - else check first non-zero bit of OTA switch
	// - write OTA switch with first non-zero bit cleared

	if (!otaHasImage1() || !otaHasImage2())
		return false;

	uint32_t value = HAL_READ32(SPI_FLASH_BASE, FLASH_SYSTEM_OFFSET + 4);
	if (value == 0) {
		uint8_t *system = (uint8_t *)malloc(64);
		Flash.readBlock(FLASH_SYSTEM_OFFSET, system, 64);
		// reset OTA switch
		((uint32_t *)system)[1] = -2;
		Flash.eraseSector(FLASH_SYSTEM_OFFSET);
		return Flash.writeBlock(FLASH_SYSTEM_OFFSET, system, 64);
	}

	uint8_t i;
	// find first non-zero bit
	for (i = 0; i < 32; i++) {
		if (value & (1 << i))
			break;
	}
	// clear the bit
	value &= ~(1 << i);
	// write OTA switch to flash
	flash_write_word(NULL, FLASH_SYSTEM_OFFSET + 4, value);
	return true;
}

/* Watchdog */

bool LibreTuya::wdtEnable(uint32_t timeout) {
	watchdog_init(timeout);
	watchdog_start();
}

void LibreTuya::wdtDisable() {
	watchdog_stop();
}

void LibreTuya::wdtFeed() {
	watchdog_refresh();
}

/* Global instance */

LibreTuya LT;
LibreTuya ESP = LT;

/* Copyright (c) Kuba Szczodrzyński 2022-05-28. */

#include <LibreTuyaAPI.h>

#include <rtl8710b.h>
#include <sys_api.h>

void LibreTuya::restart() {
	sys_reset();
}

/* CPU-related */

ChipType LibreTuya::getChipType() {
	uint8_t chipId;
	EFUSE_OneByteReadROM(9902, 0xF8, &chipId, L25EOUTVOLTAGE);
	return (ChipType)(((RTL8710B >> 24) << 8) | chipId);
}

const char *LibreTuya::getChipModel() {
	return STRINGIFY_MACRO(MCU);
}

uint32_t LibreTuya::getChipId() {
	uint32_t chipId = 0;
	uint8_t *id		= (uint8_t *)&chipId;
	// 9902 was extracted from ROM disassembly, probably not needed
	EFUSE_OneByteReadROM(9902, 0x3B, id + 0, L25EOUTVOLTAGE);
	EFUSE_OneByteReadROM(9902, 0x3C, id + 1, L25EOUTVOLTAGE);
	EFUSE_OneByteReadROM(9902, 0x3D, id + 2, L25EOUTVOLTAGE);
	return chipId;
}

uint8_t LibreTuya::getChipCores() {
	return 1;
}

const char *LibreTuya::getChipCoreType() {
	return "ARM Cortex-M4F";
}

uint32_t LibreTuya::getCpuFreqMHz() {
	return CPU_ClkGet(false) / 1000000;
}

inline uint32_t LibreTuya::getCycleCount() {
	return microsecondsToClockCycles(micros());
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

uint8_t LibreTuya::getOtaRunning() {
	uint32_t *otaAddress = (uint32_t *)0x8009000;
	if (*otaAddress == 0xFFFFFFFF)
		return 1;
	uint32_t otaCounter = *((uint32_t *)0x8009004);
	// what the-
	// "LSB 0 bits number is odd/even"
	uint8_t count = 0;
	for (uint8_t i = 0; i < 32; i++) {
		if ((otaCounter & (1 << i)) == 0)
			count++;
	}
	return 1 + (count % 2);
}

uint8_t LibreTuya::getOtaTarget() {
	return getOtaRunning() ^ 0b11;
}

bool LibreTuya::switchOta() {}

/* Global instance */

LibreTuya LT;

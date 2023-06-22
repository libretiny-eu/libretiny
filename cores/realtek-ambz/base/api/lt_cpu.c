/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#include <libretiny.h>
#include <sdk_private.h>

lt_cpu_model_t lt_cpu_get_model() {
	uint8_t chipId;
	EFUSE_OneByteReadROM(9902, 0xF8, &chipId, L25EOUTVOLTAGE);
	return CPU_MODEL_ENUM(FAMILY, chipId);
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
	return "ARM Cortex-M4F (ARMv7E-M)";
}

uint32_t lt_cpu_get_freq() {
	return CPU_ClkGet(false);
}

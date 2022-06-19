/* Copyright (c) Kuba Szczodrzyński 2022-04-24. */

#include "Flash.h"

extern "C" {
#include <fal.h>
}

// Global Flash object.
FlashClass Flash;

FlashId FlashClass::getChipId() {
	return LT.getFlashChipId();
}

uint32_t FlashClass::getSize() {
	return LT.getFlashChipSize();
}

bool FlashClass::eraseSector(uint32_t offset) {
	return fal_partition_erase(fal_root_part, offset, 1) >= 0;
}

bool FlashClass::readBlock(uint32_t offset, uint8_t *data, size_t size) {
	return fal_partition_read(fal_root_part, offset, data, size) >= 0;
}

bool FlashClass::writeBlock(uint32_t offset, uint8_t *data, size_t size) {
	return fal_partition_write(fal_root_part, offset, data, size) >= 0;
}

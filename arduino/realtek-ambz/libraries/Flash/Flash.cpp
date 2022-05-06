/* Copyright (c) Kuba Szczodrzyński 2022-04-24. */

#include "Flash.h"

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <flash_api.h>

#ifdef __cplusplus
} // extern "C"
#endif

FlashClass::FlashClass() {
	flash = NULL;
}

FlashClass::~FlashClass() {
	if (!flash)
		return;
	free(flash);
	flash = NULL;
}

void FlashClass::initialize() {
	if (flash)
		return;
	flash = (flash_t *)malloc(sizeof(flash_t));
	flash_get_status(flash);
}

FlashId FlashClass::getChipId() {
	initialize();
	FlashId id;
	uint8_t idBytes[3];
	flash_read_id(flash, idBytes, 3);
	id.manufacturerId = idBytes[0];
	id.chipId		  = idBytes[1];
	id.chipSizeId	  = idBytes[2];
	return id;
}

uint32_t FlashClass::getSize() {
	initialize();
	FlashId id = getChipId();
	if (id.chipSizeId >= 0x14 && id.chipSizeId <= 0x19) {
		return (1 << id.chipSizeId);
	}
	return 1024 * 1024;
}

bool FlashClass::eraseSector(uint32_t offset) {
	initialize();
	flash_erase_sector(flash, offset);
	return true;
}

bool FlashClass::readBlock(uint32_t offset, uint8_t *data, size_t size) {
	initialize();
	return flash_stream_read(flash, offset, size, data);
}

bool FlashClass::writeBlock(uint32_t offset, uint8_t *data, size_t size) {
	initialize();
	return flash_stream_write(flash, offset, size, data);
}

FlashClass Flash;

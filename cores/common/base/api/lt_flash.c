/* Copyright (c) Kuba Szczodrzyński 2022-04-29. */

#include "lt_flash.h"

#include <fal.h>

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

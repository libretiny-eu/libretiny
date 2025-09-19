/* Copyright (c) Etienne Le Cousin 2024-03-03. */

#include <libretiny.h>
#include <sdk_private.h>

uint32_t hal_flash_read_id(void);

lt_flash_id_t lt_flash_get_id() {
	lt_flash_id_t id;
	uint32_t fl_id	   = hal_flash_read_id();
	id.manufacturer_id = (uint8_t)(fl_id >> 16);
	id.chip_id		   = (uint8_t)(fl_id >> 8);
	id.chip_size_id	   = (uint8_t)(fl_id >> 0);
	return id;
}

/* Copyright (c) Kuba Szczodrzyński 2023-05-23. */

#include <libretiny.h>
#include <sdk_private.h>

lt_flash_id_t lt_flash_get_id() {
	lt_flash_id_t id;
	uint8_t idBytes[3];
	flash_read_id(&lt_flash_obj, idBytes, 3);
	id.manufacturer_id = idBytes[0];
	id.chip_id		   = idBytes[1];
	id.chip_size_id	   = idBytes[2];
	return id;
}

/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#include <libretiny.h>
#include <sdk_private.h>
#include <device_lock.h>

lt_flash_id_t lt_flash_get_id() {
	lt_flash_id_t id;
	uint8_t idBytes[3] = {0};
	/* Like stream reads, the SDK ID command does not acquire the device lock. */
	device_mutex_lock(RT_DEV_LOCK_FLASH);
	flash_read_id(NULL, idBytes, 3);
	device_mutex_unlock(RT_DEV_LOCK_FLASH);
	id.manufacturer_id = idBytes[0];
	id.chip_id		   = idBytes[1];
	id.chip_size_id	   = idBytes[2];
	return id;
}

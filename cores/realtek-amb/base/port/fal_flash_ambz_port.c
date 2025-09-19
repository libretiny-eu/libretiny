/* Copyright (c) Kuba Szczodrzyński 2022-05-24. */

#include <libretiny.h>
#include <sdk_private.h>

#include <fal.h>

#define FLASH_ERASE_MIN_SIZE (4 * 1024)

flash_t lt_flash_obj;

static int init() {
	flash_get_status(&lt_flash_obj);
	return 0;
}

static int read(long offset, uint8_t *buf, size_t size) {
	return size * flash_stream_read(&lt_flash_obj, offset, size, buf);
}

static int write(long offset, const uint8_t *buf, size_t size) {
	return size * flash_stream_write(&lt_flash_obj, offset, size, (uint8_t *)buf);
}

static int erase(long offset, size_t size) {
	offset &= ~(FLASH_ERASE_MIN_SIZE - 1);
	size = ((size - 1) / FLASH_ERASE_MIN_SIZE) + 1;
	for (uint16_t i = 0; i < size; i++) {
		flash_erase_sector(&lt_flash_obj, offset + i * FLASH_ERASE_MIN_SIZE);
	}
	return size * FLASH_ERASE_MIN_SIZE;
}

const struct fal_flash_dev flash0 = {
	.name		= FAL_FLASH_DEV_NAME,
	.addr		= 0x0,
	.len		= FLASH_LENGTH,
	.blk_size	= FLASH_ERASE_MIN_SIZE,
	.ops		= {init, read, write, erase},
	.write_gran = 1,
};

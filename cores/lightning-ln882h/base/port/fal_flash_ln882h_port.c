/* Copyright (c) Etienne Le Cousin 2024-02-10. */

#include <libretiny.h>
#include <sdk_private.h>

#include <fal.h>
#include <hal/hal_flash.h>

#define FLASH_ERASE_MIN_SIZE (4 * 1024)

static int init() {
	return 0;
}

static int read(long offset, uint8_t *buf, size_t size) {
	hal_flash_read(offset, size, buf);
	return size;
}

static int write(long offset, const uint8_t *buf, size_t size) {
	hal_flash_program(offset, size, (uint8_t *)buf);
	return size;
}

static int erase(long offset, size_t size) {
	offset &= ~(FLASH_ERASE_MIN_SIZE - 1);
	size = ((size - 1) / FLASH_ERASE_MIN_SIZE) + 1;
	hal_flash_erase(offset, size * FLASH_ERASE_MIN_SIZE);
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

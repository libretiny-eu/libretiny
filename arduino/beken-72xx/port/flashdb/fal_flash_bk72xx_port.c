/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <sdk_extern.h>

#include <fal.h>
#include <flash_pub.h>

#define FLASH_ERASE_MIN_SIZE (4 * 1024)

extern uint32_t flash_ctrl(uint32_t cmd, void *param);

static int init() {
	__wrap_bk_printf_disable();
	flash_init();
	__wrap_bk_printf_enable();
	return 0;
}

static int read(long offset, uint8_t *buf, size_t size) {
	flash_read((char *)buf, size, offset);
	return size;
}

static int write(long offset, const uint8_t *buf, size_t size) {
	flash_write((char *)buf, size, offset);
	return size;
}

static int erase(long offset, size_t size) {
	size = ((size - 1) / FLASH_ERASE_MIN_SIZE) + 1;
	for (uint16_t i = 0; i < size; i++) {
		uint32_t addr = offset + i * FLASH_ERASE_MIN_SIZE;
		flash_ctrl(CMD_FLASH_ERASE_SECTOR, &addr);
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

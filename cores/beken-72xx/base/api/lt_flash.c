/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#include <libretiny.h>
#include <sdk_private.h>

// can't include <flash.h> as it collides with <Flash.h> on Windows -_-
#define REG_FLASH_BASE		 0x00803000
#define REG_FLASH_OPERATE_SW (REG_FLASH_BASE + 0 * 4)
#define REG_FLASH_RDID		 (REG_FLASH_BASE + 4 * 4)
#define FLASH_BUSY_SW		 (0x01UL << 31)
#define FLASH_WP_VALUE		 (0x01UL << 30)
#define FLASH_OP_SW			 (0x01UL << 29)
#define FLASH_OP_TYPE_POS	 24
#define FLASH_OP_RDID		 20

lt_flash_id_t lt_flash_get_id() {
	uint32_t data = (FLASH_OP_RDID << FLASH_OP_TYPE_POS) | FLASH_OP_SW | FLASH_WP_VALUE;
	REG_WRITE(REG_FLASH_OPERATE_SW, data);
	while (REG_READ(REG_FLASH_OPERATE_SW) & FLASH_BUSY_SW) {}
	lt_flash_id_t id = {
		.manufacturer_id = REG_RD8(REG_FLASH_RDID, 2),
		.chip_id		 = REG_RD8(REG_FLASH_RDID, 1),
		.chip_size_id	 = REG_RD8(REG_FLASH_RDID, 0),
	};
	return id;
}

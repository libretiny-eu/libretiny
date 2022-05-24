/* Copyright (c) Kuba Szczodrzyński 2022-05-24. */

#pragma once

// Flash device configuration
extern const struct fal_flash_dev flash0;

#define FAL_FLASH_DEV_NAME "flash0"

#define FAL_FLASH_DEV_TABLE                                                                                            \
	{ &flash0, }

// Partition table
#define FAL_PART_HAS_TABLE_CFG

#define FAL_PART_TABLE                                                                                                 \
	{                                                                                                                  \
		{                                                                                                              \
			.magic_word = FAL_PART_MAGIC_WORD,                                                                         \
			.name		= "userdata",                                                                                  \
			.flash_name = FAL_FLASH_DEV_NAME,                                                                          \
			.offset		= FLASH_USERDATA_OFFSET,                                                                       \
			.len		= 0x4000,                                                                                      \
		},                                                                                                             \
	}

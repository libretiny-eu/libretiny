/* Copyright (c) Kuba Szczodrzyński 2022-05-24. */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

inline void printf_nop(const char *fmt, ...) {}

#define FAL_PRINTF printf_nop
#define FAL_DEBUG  0

// Flash device configuration
extern const struct fal_flash_dev flash0;

#ifdef __cplusplus
} // extern "C"
#endif

#define FAL_FLASH_DEV_NAME "flash0"

#define FAL_FLASH_DEV_TABLE                                                                                            \
	{ &flash0, }

#define FAL_DEV_NAME_MAX 16 // no need for 24 chars (default)

// Partition table
#define FAL_PART_HAS_TABLE_CFG

#define FAL_PART_TABLE_ITEM(part_lower, part_upper)                                                                    \
	{                                                                                                                  \
		.magic_word = FAL_PART_MAGIC_WORD,		   /* magic word */                                                    \
		.name		= #part_lower,				   /* lowercase name as string */                                      \
		.flash_name = FAL_FLASH_DEV_NAME,		   /* flash device name */                                             \
		.offset		= FLASH_##part_upper##_OFFSET, /* partition offset macro as uppercase string */                    \
		.len		= FLASH_##part_upper##_LENGTH, /* partition length macro as uppercase string */                    \
	},

// for fal_partition_t
#include <fal_def.h>

/**
 * @brief "Root" partition entry, representing the entire flash.
 */
extern fal_partition_t fal_root_part;

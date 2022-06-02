/* Copyright (c) Kuba Szczodrzyński 2022-05-28. */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <fal.h>

#define UF2_MAGIC_1 0x0A324655
#define UF2_MAGIC_2 0x9E5D5157
#define UF2_MAGIC_3 0x0AB16F30

#define UF2_BLOCK_SIZE sizeof(uf2_block_t)

typedef struct __attribute__((packed)) {
	// 32 byte header
	uint32_t magic1;
	uint32_t magic2;

	// flags split as bitfields
	bool not_main_flash : 1;
	uint16_t dummy1 : 11;
	bool file_container : 1;
	bool has_family_id : 1;
	bool has_md5 : 1;
	bool has_tags : 1;
	uint16_t dummy2 : 16;

	uint32_t addr;
	uint32_t len;
	uint32_t block_seq;
	uint32_t block_count;
	uint32_t file_size; // or familyID;
	uint8_t data[476];
	uint32_t magic3;
} uf2_block_t;

typedef struct {
	uint32_t seq; // current block sequence number

	uint8_t *binpatch;	  // current block's binpatch (if any) -> pointer inside block->data
	uint8_t binpatch_len; // binpatch length

	bool has_ota1; // image has any data for OTA1
	bool has_ota2; // image has any data for OTA2

	uint8_t ota_idx;	// target OTA index
	uint32_t family_id; // expected family ID

	uint32_t erased_offset; // offset of region erased during update
	uint32_t erased_length; // length of erased region

	fal_partition_t part1; // OTA1 target partition
	fal_partition_t part2; // OTA2 target partition
} uf2_ota_t;

typedef struct {
	char *fw_name;
	char *fw_version;
	char *lt_version;
	char *board;
} uf2_info_t;

typedef enum {
	UF2_TAG_VERSION	  = 0x9FC7BC, // version of firmware file - UTF8 semver string
	UF2_TAG_PAGE_SIZE = 0x0BE9F7, // page size of target device (32 bit unsigned number)
	UF2_TAG_SHA2	  = 0xB46DB0, // SHA-2 checksum of firmware (can be of various size)
	UF2_TAG_DEVICE	  = 0x650D9D, // description of device (UTF8)
	UF2_TAG_DEVICE_ID = 0xC8A729, // device type identifier
	// LibreTuya custom, tags
	UF2_TAG_OTA_VERSION = 0x5D57D0, // format version
	UF2_TAG_BOARD		= 0xCA25C8, // board name (lowercase code)
	UF2_TAG_FIRMWARE	= 0x00DE43, // firmware description / name
	UF2_TAG_BUILD_DATE	= 0x822F30, // build date/time as Unix timestamp
	UF2_TAG_LT_VERSION	= 0x59563D, // LT version (semver)
	UF2_TAG_PART_1		= 0x805946, // OTA1 partition name
	UF2_TAG_PART_2		= 0xA1E4D7, // OTA2 partition name
	UF2_TAG_HAS_OTA1	= 0xBBD965, // image has any data for OTA1
	UF2_TAG_HAS_OTA2	= 0x92280E, // image has any data for OTA2
	UF2_TAG_BINPATCH	= 0xB948DE, // binary patch to convert OTA1->OTA2
} uf2_tag_type_t;

typedef enum {
	UF2_OPC_DIFF32 = 0xFE,
} uf2_opcode_t;

typedef enum {
	UF2_ERR_OK = 0,
	UF2_ERR_IGNORE,		   // block should be ignored
	UF2_ERR_MAGIC,		   // wrong magic numbers
	UF2_ERR_FAMILY,		   // family ID mismatched
	UF2_ERR_NOT_HEADER,	   // block is not a header
	UF2_ERR_OTA_VER,	   // unknown/invalid OTA format version
	UF2_ERR_OTA_WRONG,	   // no data for current OTA index
	UF2_ERR_PART_404,	   // no partition with that name
	UF2_ERR_PART_ONE,	   // only one partition tag in a block
	UF2_ERR_PART_UNSET,	   // image broken - attempted to write without target partition
	UF2_ERR_DATA_TOO_LONG, // data too long - tags won't fit
	UF2_ERR_SEQ_MISMATCH,  // sequence number mismatched
	UF2_ERR_ERASE_FAILED,  // erasing flash failed
	UF2_ERR_WRITE_FAILED,  // writing to flash failed
	UF2_ERR_WRITE_LENGTH,  // wrote fewer data than requested
} uf2_err_t;

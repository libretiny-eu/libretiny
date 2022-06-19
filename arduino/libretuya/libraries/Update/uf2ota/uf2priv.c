/* Copyright (c) Kuba Szczodrzyński 2022-05-29. */

#include "uf2priv.h"

uf2_err_t uf2_parse_block(uf2_ota_t *ctx, uf2_block_t *block, uf2_info_t *info) {
	if (block->block_seq != ctx->seq)
		// sequence number must match
		return UF2_ERR_SEQ_MISMATCH;
	ctx->seq++; // increment sequence number after checking it

	if (!block->has_tags)
		// no tags in this block, no further processing needed
		return UF2_ERR_OK;

	if (block->len > (476 - 4 - 4))
		// at least one tag + last tag must fit
		return UF2_ERR_DATA_TOO_LONG;

	uint8_t *tags_start = block->data + block->len;
	uint8_t tags_len	= 476 - block->len;
	uint8_t tags_pos	= 0;
	if (block->has_md5)
		tags_len -= 24;

	ctx->binpatch_len = 0; // binpatch applies to one block only
	char *part1		  = NULL;
	char *part2		  = NULL;

	uf2_tag_type_t type;
	while (tags_pos < tags_len) {
		uint8_t len = uf2_read_tag(tags_start + tags_pos, &type);
		if (!len)
			break;
		tags_pos += 4; // skip tag header
		uint8_t *tag = tags_start + tags_pos;

		char **str_dest = NULL; // char* to copy the tag into

		switch (type) {
			case UF2_TAG_OTA_VERSION:
				if (tag[0] != 1)
					return UF2_ERR_OTA_VER;
				break;
			case UF2_TAG_FIRMWARE:
				if (info)
					str_dest = &(info->fw_name);
				break;
			case UF2_TAG_VERSION:
				if (info)
					str_dest = &(info->fw_version);
				break;
			case UF2_TAG_LT_VERSION:
				if (info)
					str_dest = &(info->lt_version);
				break;
			case UF2_TAG_BOARD:
				if (info)
					str_dest = &(info->board);
				break;
			case UF2_TAG_HAS_OTA1:
				ctx->has_ota1 = tag[0];
				break;
			case UF2_TAG_HAS_OTA2:
				ctx->has_ota2 = tag[0];
				break;
			case UF2_TAG_PART_1:
				str_dest = &(part1);
				break;
			case UF2_TAG_PART_2:
				str_dest = &(part2);
				break;
			case UF2_TAG_BINPATCH:
				ctx->binpatch	  = tag;
				ctx->binpatch_len = len;
				break;
			default:
				break;
		}

		if (str_dest) {
			*str_dest = (char *)zalloc(len + 1);
			memcpy(*str_dest, tag, len);
		}
		// align position to 4 bytes
		tags_pos += (((len - 1) / 4) + 1) * 4;
	}

	if (part1 && part2) {
		// update current target partition
		uf2_err_t err = uf2_update_parts(ctx, part1, part2);
		if (err)
			return err;
	} else if (part1 || part2) {
		// only none or both partitions can be specified
		return UF2_ERR_PART_ONE;
	}

	return UF2_ERR_OK;
}

uint8_t uf2_read_tag(const uint8_t *data, uf2_tag_type_t *type) {
	uint8_t len = data[0];
	if (!len)
		return 0;
	uint32_t tag_type = *((uint32_t *)data);
	if (!tag_type)
		return 0;
	*type = tag_type >> 8; // remove tag length byte
	return len - 4;
}

uf2_err_t uf2_update_parts(uf2_ota_t *ctx, char *part1, char *part2) {
	// reset both target partitions
	ctx->part1 = NULL;
	ctx->part2 = NULL;
	// reset offsets as they probably don't apply to this partition
	ctx->erased_offset = 0;
	ctx->erased_length = 0;

	if (part1[0]) {
		ctx->part1 = (fal_partition_t)fal_partition_find(part1);
		if (!ctx->part1)
			return UF2_ERR_PART_404;
	}
	if (part2[0]) {
		ctx->part2 = (fal_partition_t)fal_partition_find(part2);
		if (!ctx->part2)
			return UF2_ERR_PART_404;
	}

	return UF2_ERR_OK;
}

fal_partition_t uf2_get_target_part(uf2_ota_t *ctx) {
	if (ctx->ota_idx == 1)
		return ctx->part1;
	if (ctx->ota_idx == 2)
		return ctx->part2;
	return NULL;
}

bool uf2_is_erased(uf2_ota_t *ctx, uint32_t offset, uint32_t length) {
	uint32_t erased_end = ctx->erased_offset + ctx->erased_length;
	uint32_t end		= offset + length;
	return (offset >= ctx->erased_offset) && (end <= erased_end);
}

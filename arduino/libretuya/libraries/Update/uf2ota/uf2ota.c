/* Copyright (c) Kuba Szczodrzyński 2022-05-29. */

#include "uf2priv.h"

uf2_ota_t *uf2_ctx_init(uint8_t ota_idx, uint32_t family_id) {
	uf2_ota_t *ctx = (uf2_ota_t *)zalloc(sizeof(uf2_ota_t));
	ctx->ota_idx   = ota_idx;
	ctx->family_id = family_id;
	return ctx;
}

uf2_info_t *uf2_info_init() {
	uf2_info_t *info = (uf2_info_t *)zalloc(sizeof(uf2_info_t));
	return info;
}

void uf2_info_free(uf2_info_t *info) {
	if (!info)
		return;
	free(info->fw_name);
	free(info->fw_version);
	free(info->lt_version);
	free(info->board);
	free(info);
}

uf2_err_t uf2_check_block(uf2_ota_t *ctx, uf2_block_t *block) {
	if (block->magic1 != UF2_MAGIC_1)
		return UF2_ERR_MAGIC;
	if (block->magic2 != UF2_MAGIC_2)
		return UF2_ERR_MAGIC;
	if (block->magic3 != UF2_MAGIC_3)
		return UF2_ERR_MAGIC;
	if (block->file_container)
		// ignore file containers, for now
		return UF2_ERR_IGNORE;
	if (!block->has_family_id || block->file_size != ctx->family_id)
		// require family_id
		return UF2_ERR_FAMILY;
	return UF2_ERR_OK;
}

uf2_err_t uf2_parse_header(uf2_ota_t *ctx, uf2_block_t *block, uf2_info_t *info) {
	if (!block->has_tags || block->file_container || block->len)
		// header must have tags and no data
		return UF2_ERR_NOT_HEADER;

	uf2_err_t err = uf2_parse_block(ctx, block, info);
	if (err)
		return err;

	if ((ctx->ota_idx == 1 && !ctx->has_ota1) || !ctx->has_ota2)
		return UF2_ERR_OTA_WRONG;
	return UF2_ERR_OK;
}

uf2_err_t uf2_write(uf2_ota_t *ctx, uf2_block_t *block) {
	if (ctx->seq == 0)
		return uf2_parse_header(ctx, block, NULL);
	if (block->not_main_flash || !block->len)
		// ignore blocks not meant for flashing
		return UF2_ERR_IGNORE;

	uf2_err_t err = uf2_parse_block(ctx, block, NULL);
	if (err)
		return err;

	if (!ctx->part1 && !ctx->part2)
		// no partitions set at all
		return UF2_ERR_PART_UNSET;

	fal_partition_t part = uf2_get_target_part(ctx);
	if (!part)
		// image is not for current OTA scheme
		return UF2_ERR_IGNORE;

	if (ctx->ota_idx == 2 && ctx->binpatch_len) {
		// apply binpatch
		err = uf2_binpatch(block->data, ctx->binpatch, ctx->binpatch_len);
		if (err)
			return err;
	}

	int ret;
	// erase sectors if needed
	if (!uf2_is_erased(ctx, block->addr, block->len)) {
		ret = fal_partition_erase(part, block->addr, block->len);
		if (ret < 0)
			return UF2_ERR_ERASE_FAILED;
		ctx->erased_offset = block->addr;
		ctx->erased_length = ret;
	}
	// write data to flash
	ret = fal_partition_write(part, block->addr, block->data, block->len);
	if (ret < 0)
		return UF2_ERR_WRITE_FAILED;
	if (ret != block->len)
		return UF2_ERR_WRITE_LENGTH;
	return UF2_ERR_OK;
}

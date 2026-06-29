/* Copyright (c) Kuba Szczodrzyński 2022-04-29. */

#include "lt_ota.h"

#include <uf2ota/uf2ota.h>

static inline size_t lt_ota_buf_left(lt_ota_ctx_t *ctx) {
	return ctx->buf + UF2_BLOCK_SIZE - ctx->buf_pos;
}

static inline size_t lt_ota_buf_size(lt_ota_ctx_t *ctx) {
	return ctx->buf_pos - ctx->buf;
}

void lt_ota_begin(lt_ota_ctx_t *ctx, size_t size) {
	if (!ctx)
		return;

	memset(ctx, 0, sizeof(lt_ota_ctx_t));
	uf2_ctx_init(&ctx->uf2, lt_ota_get_uf2_scheme(), lt_cpu_get_family());
	uf2_info_init(&ctx->info);
	ctx->buf_pos	 = ctx->buf;
	ctx->bytes_total = size;
	ctx->running	 = true;

	lt_ota_set_write_protect(&ctx->uf2);

	LT_DM(OTA, "begin(%u, ...) / OTA curr: %u, scheme: %u", size, lt_ota_dual_get_current(), lt_ota_get_uf2_scheme());
}

bool lt_ota_end(lt_ota_ctx_t *ctx) {
	if (!ctx || !ctx->running)
		return true;

	uf2_ctx_free(&ctx->uf2);
	uf2_info_free(&ctx->info);
	ctx->running = false;

	if (ctx->bytes_written && ctx->bytes_written == ctx->bytes_total) {
		// try to activate the 2nd image
		return lt_ota_switch(/* revert= */ false);
	}

	// activation not attempted (update aborted)
	return true;
}

__attribute__((weak)) void lt_ota_set_write_protect(uf2_ota_t *uf2) {}

size_t lt_ota_write(lt_ota_ctx_t *ctx, const uint8_t *data, size_t len) {
	if (!ctx || !ctx->running)
		return 0;

	// write until buffer space is available
	size_t written = 0;
	uint16_t to_write; // 1..512
	while (len && (to_write = MIN((uint16_t)len, lt_ota_buf_left(ctx)))) {
		LT_VM(OTA, "Writing %u to buffer (%u/512)", len, lt_ota_buf_size(ctx));

		uf2_block_t *block = NULL;
		if (to_write == UF2_BLOCK_SIZE) {
			// data has a complete block; don't use the buffer
			block = (uf2_block_t *)data;
		} else {
			// data has a part of a block; append it to the buffer
			memcpy(ctx->buf_pos, data, to_write);
			ctx->buf_pos += to_write;
			if (lt_ota_buf_size(ctx) == UF2_BLOCK_SIZE) {
				// the block is complete now
				block = (uf2_block_t *)ctx->buf;
			}
		}

		// write if a block is ready
		if (block && lt_ota_write_block(ctx, block) == false)
			// return on errors
			return written;
		data += to_write;
		len -= to_write;
		written += to_write;
	}
	return written;
}

bool lt_ota_write_block(lt_ota_ctx_t *ctx, uf2_block_t *block) {
	ctx->error = uf2_check_block(&ctx->uf2, block);
	if (ctx->error > UF2_ERR_IGNORE)
		// block is invalid
		return false;

	if (!ctx->bytes_written) {
		// parse header block to allow retrieving firmware info
		ctx->error = uf2_parse_header(&ctx->uf2, block, &ctx->info);
		if (ctx->error != UF2_ERR_OK)
			return false;

		LT_IM(
			OTA,
			"%s v%s - LT v%s @ %s",
			ctx->info.fw_name,
			ctx->info.fw_version,
			ctx->info.lt_version,
			ctx->info.board
		);

		if (ctx->bytes_total == 0) {
			// set total update size from block count info
			ctx->bytes_total = block->block_count * UF2_BLOCK_SIZE;
		} else if (ctx->bytes_total != block->block_count * UF2_BLOCK_SIZE) {
			// given update size does not match the block count
			LT_EM(
				OTA,
				"Image size wrong; got %u, calculated %llu",
				ctx->bytes_total,
				block->block_count * UF2_BLOCK_SIZE
			);
			return false;
		}
	} else if (ctx->error == UF2_ERR_OK) {
		// write data blocks normally
		ctx->error = uf2_write(&ctx->uf2, block);
		if (ctx->error > UF2_ERR_IGNORE)
			// block writing failed
			return false;
	}

	// increment total writing progress
	ctx->bytes_written += UF2_BLOCK_SIZE;
	// call progress callback
	if (ctx->callback)
		ctx->callback(ctx->callback_param);
	// reset the buffer as it's used already
	if (lt_ota_buf_size(ctx) == UF2_BLOCK_SIZE)
		ctx->buf_pos = ctx->buf;

	return true;
}

bool lt_ota_can_rollback() {
	if (lt_ota_get_type() != OTA_TYPE_DUAL)
		return false;
	uint8_t current = lt_ota_dual_get_current();
	if (current == 0)
		return false;
	return lt_ota_is_valid(current ^ 0b11);
}

uf2_ota_scheme_t lt_ota_get_uf2_scheme() {
	if (lt_ota_get_type() == OTA_TYPE_SINGLE)
		return UF2_SCHEME_DEVICE_SINGLE;
	uint8_t current = lt_ota_dual_get_current();
	if (current == 0)
		return UF2_SCHEME_DEVICE_DUAL_1;
	// UF2_SCHEME_DEVICE_DUAL_1 or UF2_SCHEME_DEVICE_DUAL_2
	return (uf2_ota_scheme_t)(current ^ 0b11);
}

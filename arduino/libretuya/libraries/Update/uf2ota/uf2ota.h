/* Copyright (c) Kuba Szczodrzyński 2022-05-28. */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "uf2types.h"

/**
 * @brief Create an UF2 OTA context.
 *
 * @param ota_idx target OTA index
 * @param family_id expected family ID
 * @return uf2_ota_t* heap-allocated structure
 */
uf2_ota_t *uf2_ctx_init(uint8_t ota_idx, uint32_t family_id);

/**
 * @brief Create an UF2 Info structure.
 *
 * @return uf2_info_t* heap-allocated structure
 */
uf2_info_t *uf2_info_init();

/**
 * @brief Free values in the info structure AND the structure itself.
 *
 * @param info structure to free; may be NULL
 */
void uf2_info_free(uf2_info_t *info);

/**
 * @brief Check if block is valid.
 *
 * @param ctx context
 * @param block block to check
 * @return uf2_err_t error code; UF2_ERR_OK and UF2_ERR_IGNORE denote valid blocks
 */
uf2_err_t uf2_check_block(uf2_ota_t *ctx, uf2_block_t *block);

/**
 * @brief Parse header block (LibreTuya UF2 first block).
 *
 * Note: caller should call uf2_check_block() first.
 *
 * @param ctx context
 * @param block block to parse
 * @param info structure to write firmware info, NULL if not used
 * @return uf2_err_t error code
 */
uf2_err_t uf2_parse_header(uf2_ota_t *ctx, uf2_block_t *block, uf2_info_t *info);

/**
 * @brief Write the block to flash memory.
 *
 * Note: caller should call uf2_check_block() first.
 *
 * @param ctx context
 * @param block block to write
 * @return uf2_err_t error code
 */
uf2_err_t uf2_write(uf2_ota_t *ctx, uf2_block_t *block);

#ifdef __cplusplus
} // extern "C"
#endif

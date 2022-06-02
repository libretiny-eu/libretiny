/* Copyright (c) Kuba Szczodrzyński 2022-05-28. */

#pragma once

// include family stdlib APIs
#include <WVariant.h>

#include "uf2binpatch.h"
#include "uf2types.h"

/**
 * @brief Parse a block and extract information from tags.
 *
 * @param ctx context
 * @param block block to parse
 * @param info structure to write firmware info, NULL if not used
 * @return uf2_err_t error code
 */
uf2_err_t uf2_parse_block(uf2_ota_t *ctx, uf2_block_t *block, uf2_info_t *info);

/**
 * @brief Parse a tag.
 *
 * @param data pointer to tag header beginning
 * @param type [out] parsed tag type
 * @return uint8_t parsed tag data length (excl. header); 0 if invalid/last tag
 */
uint8_t uf2_read_tag(const uint8_t *data, uf2_tag_type_t *type);

/**
 * @brief Update destination partitions in context.
 *
 * Partition names cannot be NULL.
 *
 * Returns UF2_ERR_IGNORE if specified partitions don't match the
 * current OTA index.
 *
 * @param ctx context
 * @param part1 partition 1 name or empty string
 * @param part2 partition 2 name or empty string
 * @return uf2_err_t error code
 */
uf2_err_t uf2_update_parts(uf2_ota_t *ctx, char *part1, char *part2);

/**
 * @brief Get target flashing partition, depending on OTA index.
 *
 * @param ctx context
 * @return fal_partition_t target partition or NULL if not set
 */
fal_partition_t uf2_get_target_part(uf2_ota_t *ctx);

/**
 * Check if specified flash memory region was already erased during update.
 *
 * @param ctx context
 * @param offset offset to check
 * @param length length to check
 * @return bool true/false
 */
bool uf2_is_erased(uf2_ota_t *ctx, uint32_t offset, uint32_t length);

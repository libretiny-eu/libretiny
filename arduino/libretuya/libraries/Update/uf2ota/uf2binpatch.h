/* Copyright (c) Kuba Szczodrzyński 2022-05-29. */

#pragma once

#include "uf2types.h"

/**
 * @brief Apply binary patch to data.
 *
 * @param data input data
 * @param data_len input data length
 * @param binpatch binary patch data
 * @param binpatch_len binary patch data length
 * @return uf2_err_t error code
 */
uf2_err_t uf2_binpatch(uint8_t *data, const uint8_t *binpatch, uint8_t binpatch_len);

/**
 * Apply DIFF32 binary patch.
 *
 * @param data input data
 * @param len input data length
 * @param patch patch data, incl. length byte
 * @return uf2_err_t error code
 */
void uf2_binpatch_diff32(uint8_t *data, const uint8_t *patch);

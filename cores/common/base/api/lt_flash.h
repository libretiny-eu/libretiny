/* Copyright (c) Kuba Szczodrzyński 2023-03-09. */

#pragma once

#include <libretiny.h>

/**
 * @brief Flash chip ID structure.
 */
typedef struct {
	uint8_t manufacturer_id;
	uint8_t chip_id;
	uint8_t chip_size_id;
} lt_flash_id_t;

/**
 * @brief Read flash chip ID and return a lt_flash_id_t struct.
 */
lt_flash_id_t lt_flash_get_id();

/**
 * @brief Get flash chip total size.
 *
 * The default implementation uses the least significant
 * byte of the chip ID to determine the size.
 */
uint32_t lt_flash_get_size();

/**
 * @brief Erase flash area. Flash can only be erased in blocks (usually 4 KiB).
 *
 * @param offset starting offset to erase (in bytes); must be multiple of the flash chip's block size
 * @param length length of data to erase (in bytes); will be rounded up to block size
 * @return whether erasing was successful
 */
bool lt_flash_erase(uint32_t offset, size_t length);

/**
 * @brief Erase a single block of flash (usually 4 KiB).
 *
 * @param offset offset of the block (in bytes); must be multiple of the flash chip's block size
 * @return whether erasing was successful
 */
bool lt_flash_erase_block(uint32_t offset);

/**
 * @brief Read data from the flash.
 *
 * @param offset starting offset (in bytes)
 * @param data pointer to where to store the data
 * @param length length of data to read
 * @return length of data successfully read (should equal 'length')
 */
uint32_t lt_flash_read(uint32_t offset, uint8_t *data, size_t length);

/**
 * @brief Write data to the flash.
 *
 * @param offset starting offset (in bytes)
 * @param data pointer to data to write
 * @param length length of data to write
 * @return length of data successfully written (should equal 'length')
 */
uint32_t lt_flash_write(uint32_t offset, const uint8_t *data, size_t length);

/* Copyright (c) Kuba Szczodrzyński 2022-04-24. */

#pragma once

#include <Arduino.h>

#ifdef __cplusplus

class FlashClass {
  public:
	/** 
	 * @brief Flash chip ID structure.
	  */
	inline FlashId getChipId() { return lt_flash_get_id(); }

	/** 
	 * @brief Get flash chip total size.
	 *
	 * The default implementation uses the least significant
	 * byte of the chip ID to determine the size.
	  */
	inline uint32_t getSize() { return lt_flash_get_size(); }

	/** 
	 * @brief Erase a single block of flash (usually 4 KiB).
	 *
	 * @param offset offset of the block (in bytes); must be multiple of the flash chip's block size
	 * @return whether erasing was successful
	  */
	inline bool eraseSector(uint32_t offset) {
		//
		return lt_flash_erase_block(offset);
	}

	/** 
	 * @brief Read data from the flash.
	 *
	 * @param offset starting offset (in bytes)
	 * @param data pointer to where to store the data
	 * @param length length of data to read
	 * @return length of data successfully read (should equal 'length')
	  */
	inline bool readBlock(uint32_t offset, uint8_t *data, size_t length) {
		//
		return lt_flash_read(offset, data, length) == length;
	}

	/** 
	 * @brief Write data to the flash.
	 *
	 * @param offset starting offset (in bytes)
	 * @param data pointer to data to write
	 * @param length length of data to write
	 * @return length of data successfully written (should equal 'length')
	  */
	inline bool writeBlock(uint32_t offset, const uint8_t *data, size_t length) {
		//
		return lt_flash_write(offset, data, length) == length;
	}
};

extern FlashClass Flash;

#endif

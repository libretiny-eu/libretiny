/* Copyright (c) Kuba Szczodrzyński 2022-04-24. */

#pragma once

#include <Arduino.h>

#ifdef __cplusplus

class FlashClass {
  public:
	/** @copydoc lt_flash_get_id() */
	inline FlashId getChipId() { return lt_flash_get_id(); }

	/** @copydoc lt_flash_get_size() */
	inline uint32_t getSize() { return lt_flash_get_size(); }

	/** @copydoc lt_flash_erase_block() */
	inline bool eraseSector(uint32_t offset) {
		//
		return lt_flash_erase_block(offset);
	}

	/** @copydoc lt_flash_read() */
	inline bool readBlock(uint32_t offset, uint8_t *data, size_t length) {
		//
		return lt_flash_read(offset, data, length) == length;
	}

	/** @copydoc lt_flash_write() */
	inline bool writeBlock(uint32_t offset, const uint8_t *data, size_t length) {
		//
		return lt_flash_write(offset, data, length) == length;
	}
};

extern FlashClass Flash;

#endif

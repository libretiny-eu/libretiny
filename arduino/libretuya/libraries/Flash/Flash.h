/* Copyright (c) Kuba Szczodrzyński 2022-04-24. */

#pragma once

#include <Arduino.h>

class FlashClass {
  public:
	FlashId getChipId();
	uint32_t getSize();

	bool eraseSector(uint32_t offset);
	bool readBlock(uint32_t offset, uint8_t *data, size_t size);
	bool writeBlock(uint32_t offset, uint8_t *data, size_t size);
};

extern FlashClass Flash;

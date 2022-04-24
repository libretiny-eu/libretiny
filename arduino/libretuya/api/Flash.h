#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
	uint8_t manufacturerId;
	uint8_t chipId;
	uint8_t chipSizeId;
} FlashId;

class IFlashClass {
  public:
	IFlashClass() {}

	~IFlashClass() {}

	virtual FlashId getChipId() = 0;
	virtual uint32_t getSize()	= 0;

	virtual bool eraseSector(uint32_t sector)							 = 0;
	virtual bool readBlock(uint32_t offset, uint8_t *data, size_t size)	 = 0;
	virtual bool writeBlock(uint32_t offset, uint8_t *data, size_t size) = 0;
};

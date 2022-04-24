#pragma once

#include "api/Flash.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <flash_api.h>

#ifdef __cplusplus
} // extern "C"
#endif

class FlashClass : public IFlashClass {
  private:
	flash_t *flash;
	void initialize();

  public:
	FlashClass();
	~FlashClass();

	FlashId getChipId();
	uint32_t getSize();

	bool eraseSector(uint32_t offset);
	bool readBlock(uint32_t offset, uint8_t *data, size_t size);
	bool writeBlock(uint32_t offset, uint8_t *data, size_t size);
};

extern FlashClass Flash;

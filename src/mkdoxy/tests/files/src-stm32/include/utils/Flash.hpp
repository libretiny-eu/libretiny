#pragma once

#include <stdlib.h>

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_flash.h"

static constexpr uint32_t FLASH_PAGE_POWER_CALIBRATION
    = FLASH_BANK1_END + 1 - FLASH_PAGE_SIZE;

inline void flashWritePage(uint32_t pageAddress, uint8_t* data, size_t size) {
    if (pageAddress % FLASH_PAGE_SIZE != 0 || pageAddress < FLASH_BASE
        || pageAddress >= FLASH_BANK1_END) {
        abort();
    }

    if (size > FLASH_PAGE_SIZE || size % 2 != 0) {
        abort();
    }

    if (HAL_FLASH_Unlock() != HAL_OK)
        abort();

    FLASH_EraseInitTypeDef eraseCfg = {
        .TypeErase = FLASH_TYPEERASE_PAGES,
        .Banks = FLASH_BANK_1,
        .PageAddress = pageAddress,
        .NbPages = 1,
    };
    uint32_t pageError = 0;
    if (HAL_FLASHEx_Erase(&eraseCfg, &pageError) != HAL_OK)
        abort();

    uint32_t address = pageAddress;
    uint8_t* end = data + size;

    while (end - data >= 8) {
        if (HAL_FLASH_Program(
                FLASH_TYPEPROGRAM_DOUBLEWORD, address, *(uint64_t*)data)
            != HAL_OK)
            abort();
        address += 8;
        data += 8;
    }

    while (end - data >= 2) {
        if (HAL_FLASH_Program(
                FLASH_TYPEPROGRAM_DOUBLEWORD, address, *(uint16_t*)data)
            != HAL_OK)
            abort();
        address += 2;
        data += 2;
    }

    if (HAL_FLASH_Lock() != HAL_OK)
        abort();
}

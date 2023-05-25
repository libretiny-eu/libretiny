/* Copyright (c) Kuba Szczodrzyński 2023-05-23. */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// SDK
extern hal_uart_adapter_t log_uart;
void software_reset();
void sys_swd_off();
void sys_uart_download_mode();
void sys_download_mode(uint8_t mode);

// blobs
bool Hal_GetPhyEfuseMACAddr(uint32_t xFFFFE5BB, uint8_t *buf);
bool rtw_efuse_map_read(uint32_t xFFFFE5BB, uint32_t offset, uint32_t length, uint8_t *buf);
uint32_t efuse_OneByteRead(uint32_t x33300000, uint32_t addr, uint8_t *buf, uint32_t ldo);
uint32_t EFUSERead8(uint32_t x33300000, uint32_t addr, uint8_t *buf, uint32_t ldo);
uint32_t hal_get_chip_id(uint32_t *id);

#ifdef __cplusplus
} // extern "C"
#endif

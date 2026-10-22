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

// bootloader exported info
typedef struct fw_img_export_info_type_s {
	uint32_t img1_start_offset;
	uint32_t fw1_start_offset;
	uint32_t fw2_start_offset;
	uint32_t fw1_sn;
	uint32_t fw2_sn;
	uint8_t fw1_valid;
	uint8_t fw2_valid;
	uint8_t loaded_fw_idx;
} fw_img_export_info_type_t;

// ROM stubs at 0x770
typedef struct hal_flash_boot_stubs_s {
	void *ppartition_tbl;
	void *(*get_fw1_key_tbl)();
	void *(*get_fw2_key_tbl)();
	void (*clear_export_partition_tbl)();
	void (*erase_boot_loader)();
	fw_img_export_info_type_t *(*fw_img_info_tbl_query)();
	int32_t (*otu_fw_download)(hal_uart_adapter_t *potu_uart, uint32_t flash_sel, uint32_t flash_offset);
} hal_flash_boot_stubs_t;

extern const hal_flash_boot_stubs_t hal_flash_boot_stubs;

#ifdef __cplusplus
} // extern "C"
#endif

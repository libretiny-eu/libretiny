/* Copyright (c) Kuba Szczodrzyński 2022-07-07. */

#include "BkDriverFlash.h"
#include "drv_model_pub.h"
#include "uart_pub.h"

static const bk_logic_partition_t bk7231_partitions[BK_PARTITION_MAX] = {
	{
		.partition_owner	   = BK_FLASH_EMBEDDED,
		.partition_description = "Bootloader",
		.partition_start_addr  = FLASH_BOOTLOADER_OFFSET,
		.partition_length	   = FLASH_BOOTLOADER_LENGTH,
		.partition_options	   = PAR_OPT_READ_EN | PAR_OPT_WRITE_DIS,
	 },
	{
		.partition_owner	   = BK_FLASH_EMBEDDED,
		.partition_description = "Application",
		.partition_start_addr  = FLASH_APP_OFFSET,
		.partition_length	   = FLASH_APP_LENGTH,
		.partition_options	   = PAR_OPT_READ_EN | PAR_OPT_WRITE_DIS,
	 },
	{
		.partition_owner	   = BK_FLASH_EMBEDDED,
		.partition_description = "ota",
		.partition_start_addr  = FLASH_DOWNLOAD_OFFSET,
		.partition_length	   = FLASH_DOWNLOAD_LENGTH,
		.partition_options	   = PAR_OPT_READ_EN | PAR_OPT_WRITE_DIS,
	 },
	{
		.partition_owner	   = BK_FLASH_EMBEDDED,
		.partition_description = "RF Firmware",
		.partition_start_addr  = FLASH_CALIBRATION_OFFSET,
		.partition_length	   = FLASH_CALIBRATION_LENGTH,
		.partition_options	   = PAR_OPT_READ_EN | PAR_OPT_WRITE_DIS,
	 },
	{
		.partition_owner	   = BK_FLASH_EMBEDDED,
		.partition_description = "NET info",
		.partition_start_addr  = FLASH_TLV_OFFSET,
		.partition_length	   = FLASH_TLV_LENGTH,
		.partition_options	   = PAR_OPT_READ_EN | PAR_OPT_WRITE_DIS,
	 },
};

bk_logic_partition_t *__wrap_bk_flash_get_info(bk_partition_t partition) {
	if ((partition >= BK_PARTITION_BOOTLOADER) && (partition < BK_PARTITION_MAX))
		return (bk_logic_partition_t *)&bk7231_partitions[partition];
	return NULL;
}

OSStatus __wrap_bk_flash_erase(bk_partition_t partition, uint32_t off_set, uint32_t size) {
	uint32_t i;
	uint32_t param;
	UINT32 status;
	DD_HANDLE flash_hdl;
	uint32_t start_sector, end_sector;
	bk_logic_partition_t *partition_info;
	GLOBAL_INT_DECLARATION();
	partition_info = bk_flash_get_info(partition);
	start_sector   = off_set >> 12;
	end_sector	   = (off_set + size - 1) >> 12;
	flash_hdl	   = ddev_open(FLASH_DEV_NAME, &status, 0);
	ASSERT(DD_HANDLE_UNVALID != flash_hdl);
	for (i = start_sector; i <= end_sector; i++) {
		param = partition_info->partition_start_addr + (i << 12);
		GLOBAL_INT_DISABLE();
		ddev_control(flash_hdl, CMD_FLASH_ERASE_SECTOR, (void *)&param);
		GLOBAL_INT_RESTORE();
	}
	return kNoErr;
}

OSStatus
__wrap_bk_flash_write(bk_partition_t partition, volatile uint32_t off_set, uint8_t *inBuffer, uint32_t inBufferLength) {
	UINT32 status;
	DD_HANDLE flash_hdl;
	uint32_t start_addr;
	bk_logic_partition_t *partition_info;
	GLOBAL_INT_DECLARATION();
	if (NULL == inBuffer) {
		os_printf("%s inBuffer=NULL\r\n", __FUNCTION__);
		return kParamErr;
	}
	partition_info = bk_flash_get_info(partition);
	if (NULL == partition_info) {
		os_printf("%s partiion not found\r\n", __FUNCTION__);
		return kNotFoundErr;
	}
	start_addr = partition_info->partition_start_addr + off_set;
	flash_hdl  = ddev_open(FLASH_DEV_NAME, &status, 0);
	if (DD_HANDLE_UNVALID == flash_hdl) {
		os_printf("%s open failed\r\n", __FUNCTION__);
		return kOpenErr;
	}
	GLOBAL_INT_DISABLE();
	ddev_write(flash_hdl, (char *)inBuffer, inBufferLength, start_addr);
	GLOBAL_INT_RESTORE();
	return kNoErr;
}

OSStatus
__wrap_bk_flash_read(bk_partition_t partition, volatile uint32_t off_set, uint8_t *outBuffer, uint32_t inBufferLength) {
	UINT32 status;
	uint32_t start_addr;
	DD_HANDLE flash_hdl;
	bk_logic_partition_t *partition_info;
	GLOBAL_INT_DECLARATION();
	if (NULL == outBuffer) {
		os_printf("%s outBuffer=NULL\r\n", __FUNCTION__);
		return kParamErr;
	}
	partition_info = bk_flash_get_info(partition);
	if (NULL == partition_info) {
		os_printf("%s partiion not found\r\n", __FUNCTION__);
		return kNotFoundErr;
	}
	start_addr = partition_info->partition_start_addr + off_set;
	flash_hdl  = ddev_open(FLASH_DEV_NAME, &status, 0);
	if (DD_HANDLE_UNVALID == flash_hdl) {
		os_printf("%s open failed\r\n", __FUNCTION__);
		return kOpenErr;
	}
	GLOBAL_INT_DISABLE();
	ddev_read(flash_hdl, (char *)outBuffer, inBufferLength, start_addr);
	GLOBAL_INT_RESTORE();
	return kNoErr;
}

/* Copyright (c) Kuba Szczodrzyński 2023-06-20. */

#pragma once

#define GPIO_SD1_DMA_MODULE		  GPIO_SD_DMA_MODULE
#define SARADC_ADC_SATURATION_CFG (SARADC_BASE + 3 * 4)
#define SARADC_ADC_SAT_CTRL_MASK  (0x3)
#define SARADC_ADC_DAT_AFTER_STA  SARADC_ADC_DATA

#define CMD_GET_SCTRL_RETETION 0xC123F48
#define CMD_SET_SCTRL_RETETION 0xC123F49

inline void turnon_PA_in_temp_dect() {}

inline void turnoff_PA_in_temp_dect() {}

inline int if_ble_sleep() {
	return 1;
}

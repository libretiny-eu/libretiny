/* Copyright (c) Etienne Le Cousin 2025-05-04. */

#include <sdk_private.h>

void cal_adc_init(void)
{
	adc_init_t_def adc_init;

	memset(&adc_init, 0, sizeof(adc_init_t_def));
	adc_init.adc_ch  = ADC_CH0;
	adc_init.adc_conv_mode = ADC_CONV_MODE_CONTINUE;
	adc_init.adc_presc = 0xFF;
	adc_init.adc_ov_smp_ratio = ADC_OVER_SAMPLING_RATIO_X8;
	adc_init.adc_ov_smp_ratio_en = ADC_OVER_SAMPLING_EN_STATUS_BIT0;
	hal_adc_init(ADC_BASE, &adc_init);

	hal_adc_en(ADC_BASE, HAL_ENABLE);

	hal_adc_start_conv(ADC_BASE);
}

uint16_t cal_adc_read(adc_ch_t ch)
{
	uint16_t read_adc = 0;

	while(hal_adc_get_conv_status(ADC_BASE, ch) == 0);

	read_adc = hal_adc_get_data(ADC_BASE, ch);

	hal_adc_clr_conv_status(ADC_BASE,ch);

	return read_adc;
}

void cal_temp_app_task_entry()
{
	int8_t cap_comp = 0;
	uint16_t adc_val = 0;

	if (NVDS_ERR_OK == ln_nvds_get_xtal_comp_val((uint8_t *)&cap_comp)) {
		if ((uint8_t)cap_comp == 0xFF) {
			cap_comp = 0;
		}
	}

	cal_adc_init();

	wifi_temp_cal_init(cal_adc_read(ADC_CH0), cap_comp);

	while (1)
	{
		OS_MsDelay(1000);

		adc_val = cal_adc_read(ADC_CH0);
		wifi_do_temp_cal_period(adc_val);
	}
}

/* Copyright (c) Etienne Le Cousin 2025-01-19. */

#include "wiring_private.h"

#ifndef PIN_ADC1
#define PIN_ADC1 -1
#endif
#ifndef PIN_ADC2
#define PIN_ADC2 -1
#endif
#ifndef PIN_ADC3
#define PIN_ADC3 -1
#endif
#ifndef PIN_ADC4
#define PIN_ADC4 -1
#endif
#ifndef PIN_ADC5
#define PIN_ADC5 -1
#endif
#ifndef PIN_ADC6
#define PIN_ADC6 -1
#endif
#ifndef PIN_ADC7
#define PIN_ADC7 -1
#endif

const uint32_t adc_channels[] = {
	-1,
	PIN_ADC1,
	PIN_ADC2,
	PIN_ADC3,
	PIN_ADC4,
	PIN_ADC5,
	PIN_ADC6,
	PIN_ADC7,
};

static adc_ch_t pinToAdcCh(uint32_t gpio) {
	for (uint8_t i = 0; i < ADC_CH_NUM; i++) {
		if (adc_channels[i] == gpio)
			return (adc_ch_t)(1 << i);
	}
	return ADC_CH0;
}

uint16_t analogReadVoltage(pin_size_t pinNumber) {
	uint16_t ret = 0;
	pinCheckGetInfo(pinNumber, PIN_ADC, 0);

	hal_gpio_pin_mode_set(GPIO_GET_BASE(pin->gpio), GPIO_GET_PIN(pin->gpio), GPIO_MODE_ANALOG);

	adc_ch_t ch = pinToAdcCh(pin->gpio);
	adc_init_t_def adc_init;

	memset(&adc_init, 0, sizeof(adc_init_t_def));
	adc_init.adc_ch				 = ch;
	adc_init.adc_conv_mode		 = ADC_CONV_MODE_CONTINUE;
	adc_init.adc_presc			 = 80;
	adc_init.adc_ov_smp_ratio	 = ADC_OVER_SAMPLING_RATIO_X8;
	adc_init.adc_ov_smp_ratio_en = ADC_OVER_SAMPLING_EN_STATUS_BIT0;
	hal_adc_init(ADC_BASE, &adc_init);

	hal_adc_en(ADC_BASE, HAL_ENABLE);

	hal_adc_start_conv(ADC_BASE);

	while (hal_adc_get_conv_status(ADC_BASE, ch) == 0)
		;

	ret = hal_adc_get_data(ADC_BASE, ch);

	hal_adc_clr_conv_status(ADC_BASE, ch);

	return (uint16_t)(3700UL * ret / 4095);
}

uint16_t analogReadMaxVoltage(pin_size_t pinNumber) {
	return 3300;
}

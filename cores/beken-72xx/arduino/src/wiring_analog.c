/* Copyright (c) Kuba Szczodrzyński 2022-06-20. */

#include "wiring_private.h"

static GPIO_INDEX pwmToGpio[] = {
	GPIO6,	// PWM0
	GPIO7,	// PWM1
	GPIO8,	// PWM2
	GPIO9,	// PWM3
	GPIO24, // PWM4
	GPIO26, // PWM5
};

#if CFG_SOC_NAME == SOC_BK7231N
static GPIO_INDEX adcToGpio[] = {
	-1,		 // ADC0 - VBAT
	GPIONUM, // ADC1
	GPIONUM, // ADC2
	GPIO23,	 // ADC3
	GPIONUM, // ADC4
	GPIONUM, // ADC5
	GPIONUM, // ADC6
	GPIONUM, // ADC7
};
#else
static GPIO_INDEX adcToGpio[] = {
	-1,		// ADC0 - VBAT
	GPIO4,	// ADC1
	GPIO5,	// ADC2
	GPIO23, // ADC3
	GPIO2,	// ADC4
	GPIO3,	// ADC5
	GPIO12, // ADC6
	GPIO13, // ADC7
};
#endif

static uint8_t gpioToPwm(GPIO_INDEX gpio) {
	for (uint8_t i = 0; i < sizeof(pwmToGpio) / sizeof(GPIO_INDEX); i++) {
		if (pwmToGpio[i] == gpio)
			return i;
	}
	return 0;
}

static uint8_t gpioToAdc(GPIO_INDEX gpio) {
	for (uint8_t i = 0; i < sizeof(adcToGpio) / sizeof(GPIO_INDEX); i++) {
		if (adcToGpio[i] == gpio)
			return i;
	}
	return 0;
}

static uint16_t adcData[1];

uint16_t analogReadVoltage(pin_size_t pinNumber) {
	pinCheckGetInfo(pinNumber, PIN_ADC, 0);

	UINT32 status;
	saradc_desc_t adc;
	DD_HANDLE handle;
	saradc_config_param_init(&adc);
	adc.channel		   = gpioToAdc(pin->gpio);
	adc.mode		   = ADC_CONFIG_MODE_CONTINUE;
	adc.pData		   = adcData;
	adc.data_buff_size = 1;
	handle			   = ddev_open(SARADC_DEV_NAME, &status, (uint32_t)&adc);
	if (status)
		return 0;
	// wait for data
	while (!adc.has_data || adc.current_sample_data_cnt < 1) {
		delay(1);
	}
	ddev_control(handle, SARADC_CMD_RUN_OR_STOP_ADC, (void *)false);
	ddev_close(handle);
	return adcData[0];
}

uint16_t analogReadMaxVoltage(pin_size_t pinNumber) {
	return 3300;
}

void analogWrite(pin_size_t pinNumber, int value) {
	pinCheckGetData(pinNumber, PIN_PWM, );

	// GPIO can't be used together with PWM
	pinRemoveMode(pin, PIN_GPIO | PIN_IRQ);

	float percent	   = value * 1.0 / ((1 << _analogWriteResolution) - 1);
	uint32_t frequency = 26 * _analogWritePeriod - 1;
	uint32_t dutyCycle = percent * frequency;
	data->pwm.channel  = gpioToPwm(pin->gpio);
	uint32_t channel   = data->pwm.channel;
#if CFG_SOC_NAME != SOC_BK7231N
	data->pwm.duty_cycle = dutyCycle;
#else
	data->pwm.duty_cycle1 = dutyCycle;
	data->pwm.duty_cycle2 = 0;
	data->pwm.duty_cycle3 = 0;
#endif

	if (dutyCycle) {
		if (!pinEnabled(pin, PIN_PWM)) {
			// enable PWM and set its value
			data->pwm.cfg.bits.en	  = PWM_ENABLE;
			data->pwm.cfg.bits.int_en = PWM_INT_DIS;
			data->pwm.cfg.bits.mode	  = PWM_PWM_MODE;
			data->pwm.cfg.bits.clk	  = PWM_CLK_26M;
			data->pwm.end_value		  = frequency;
			data->pwm.p_Int_Handler	  = NULL;
			__wrap_bk_printf_disable();
			sddev_control(PWM_DEV_NAME, CMD_PWM_INIT_PARAM, &data->pwm);
			sddev_control(PWM_DEV_NAME, CMD_PWM_INIT_LEVL_SET_HIGH, &channel);
			sddev_control(PWM_DEV_NAME, CMD_PWM_UNIT_ENABLE, &channel);
			__wrap_bk_printf_enable();
			pinEnable(pin, PIN_PWM);
		} else {
			sddev_control(PWM_DEV_NAME, CMD_PWM_INIT_LEVL_SET_HIGH, &channel);
			// update duty cycle
			sddev_control(PWM_DEV_NAME, CMD_PWM_SET_DUTY_CYCLE, &data->pwm);
		}
	} else {
		if (pinEnabled(pin, PIN_PWM)) {
			// disable PWM
			pinRemoveMode(pin, PIN_PWM);
		}
		// force level as LOW
		pinMode(pinNumber, OUTPUT);
		digitalWrite(pinNumber, LOW);
	}
}

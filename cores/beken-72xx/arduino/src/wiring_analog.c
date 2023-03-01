/* Copyright (c) Kuba Szczodrzyński 2022-06-20. */

#include <Arduino.h>

#include <gpio_pub.h>
#include <pwm_pub.h>
#include <saradc_pub.h>

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

static pwm_param_t pwm;
static uint16_t adcData[1];

uint16_t analogReadVoltage(pin_size_t pinNumber) {
	PinInfo *pin = pinInfo(pinNumber);
	if (!pin)
		return 0;
	if (!pinSupported(pin, PIN_ADC))
		return 0;

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
	PinInfo *pin = pinInfo(pinNumber);
	if (!pin)
		return;
	if (!pinSupported(pin, PIN_PWM))
		return;

	float percent	   = value * 1.0 / ((1 << _analogWriteResolution) - 1);
	uint32_t frequency = 26 * _analogWritePeriod - 1;
	uint32_t dutyCycle = percent * frequency;
	pwm.channel		   = gpioToPwm(pin->gpio);
#if CFG_SOC_NAME != SOC_BK7231N
	pwm.duty_cycle = dutyCycle;
#else
	pwm.duty_cycle1 = dutyCycle;
	pwm.duty_cycle2 = 0;
	pwm.duty_cycle3 = 0;
#endif

	if (dutyCycle) {
		if (!pinEnabled(pin, PIN_PWM)) {
			// enable PWM and set its value
			pwm.cfg.bits.en		= PWM_ENABLE;
			pwm.cfg.bits.int_en = PWM_INT_DIS;
			pwm.cfg.bits.mode	= PWM_PWM_MODE;
			pwm.cfg.bits.clk	= PWM_CLK_26M;
			pwm.end_value		= frequency;
			pwm.p_Int_Handler	= NULL;
			__wrap_bk_printf_disable();
			sddev_control(PWM_DEV_NAME, CMD_PWM_INIT_PARAM, &pwm);
			sddev_control(PWM_DEV_NAME, CMD_PWM_INIT_LEVL_SET_HIGH, &pwm.channel);
			sddev_control(PWM_DEV_NAME, CMD_PWM_UNIT_ENABLE, &pwm.channel);
			__wrap_bk_printf_enable();
			pin->enabled &= ~PIN_GPIO;
			pin->enabled |= PIN_PWM;
		} else {
			// update duty cycle
			sddev_control(PWM_DEV_NAME, CMD_PWM_SET_DUTY_CYCLE, &pwm);
		}
	} else {
		if (pinEnabled(pin, PIN_PWM)) {
			// disable PWM
			pwm.cfg.bits.en = PWM_DISABLE;
			__wrap_bk_printf_disable();
			sddev_control(PWM_DEV_NAME, CMD_PWM_DEINIT_PARAM, &pwm);
			__wrap_bk_printf_enable();
			pin->enabled &= ~PIN_PWM;
		}
		// force level as LOW
		pinMode(pinNumber, OUTPUT);
		digitalWrite(pinNumber, LOW);
	}
}

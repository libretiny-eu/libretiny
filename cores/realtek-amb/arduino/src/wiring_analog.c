/* Copyright (c) Kuba Szczodrzyński 2022-06-20. */

#include "wiring_private.h"

/* ADC */
static analogin_t adc1;
static analogin_t adc2;
static analogin_t adc3;

static bool g_adc_enabled[] = {false, false, false};
// from realtek_amebaz_va0_example/example_sources/adc_vbat/src/main.c
#define AD2MV(ad, offset, gain) (((ad >> 4) - offset) * 1000 / gain)

// TODO implement custom ADC calibration

uint16_t analogReadVoltage(pin_size_t pinNumber) {
	uint16_t ret = 0;
	switch (pinNumber) {
#ifdef PIN_ADC2
		case PIN_ADC2:
			if (g_adc_enabled[1] == false) {
				analogin_init(&adc2, AD_2);
				g_adc_enabled[1] = true;
			}
			ret = analogin_read_u16(&adc2);
			// AD_1 - 0.0V-5.0V
			return AD2MV(ret, 0x496, 0xBA);
#endif
#ifdef PIN_ADC1
		case PIN_ADC1:
			if (g_adc_enabled[0] == false) {
				analogin_init(&adc1, AD_1);
				g_adc_enabled[0] = true;
			}
			ret = analogin_read_u16(&adc1);
			break;
#endif
#ifdef PIN_ADC3
		case PIN_ADC3:
			if (g_adc_enabled[2] == false) {
				analogin_init(&adc3, AD_3);
				g_adc_enabled[2] = true;
			}
			ret = analogin_read_u16(&adc3);
			break;
#endif
		default:
			return 0;
	}

	// AD_0, AD_2 - 0.0V-3.3V
	return AD2MV(ret, 0x418, 0x342);
}

uint16_t analogReadMaxVoltage(pin_size_t pinNumber) {
#ifdef PIN_A1
	if (pinNumber == PIN_A1)
		return 5000;
#endif
	return 3300;
}

void analogWrite(pin_size_t pinNumber, int value) {
	pinCheckGetData(pinNumber, PIN_PWM, );

	// GPIO can't be used together with PWM
	pinRemoveMode(pin, PIN_GPIO | PIN_IRQ);

	pwmout_t *pwm = data->pwm;
	if (!pwm) {
		// allocate memory if pin not used before
		data->pwm = pwm = malloc(sizeof(pwmout_t));
		pwmout_init(pwm, pin->gpio);
		pwmout_period_us(pwm, _analogWritePeriod);
		pinEnable(pin, PIN_PWM);
	}

	float percent = value * 1.0 / (1 << _analogWriteResolution);
	pwmout_write(pwm, percent);
}

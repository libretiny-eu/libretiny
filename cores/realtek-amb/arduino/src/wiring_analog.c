/* Copyright (c) Kuba Szczodrzyński 2022-06-20. */

#include <ArduinoPrivate.h>

#if LT_RTL8720D

/* ADC - AmebaD: the measurable channels are PB1/PB2/PB3 (AD_4/5/6); the AmebaZ-named
 * AD_1..AD_3 pins are not bonded out on the BW16. Uses the efuse-calibrated
 * ADC_GetVoltage(), one channel at a time. */
extern int32_t ADC_GetVoltage(uint32_t chan_data);

static analogin_t lt_adc;
static PinName lt_adc_pin = NC;

uint16_t analogReadVoltage(pin_size_t pinNumber) {
	PinName ad;
	switch (pinNumber) {
		case PIN_A0:
			ad = AD_4; // PB1
			break;
		case PIN_A1:
			ad = AD_5; // PB2
			break;
		case PIN_A2:
			ad = AD_6; // PB3
			break;
		default:
			return 0;
	}
	if (lt_adc_pin != ad) {
		if (lt_adc_pin != NC)
			analogin_deinit(&lt_adc);
		analogin_init(&lt_adc, ad);
		lt_adc_pin = ad;
	}
	uint16_t raw = analogin_read_u16(&lt_adc);
	int32_t mv	 = ADC_GetVoltage(raw);
	return mv < 0 ? 0 : (uint16_t)mv;
}

uint16_t analogReadMaxVoltage(pin_size_t pinNumber) {
	return 3300;
}

#else

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

#endif // LT_RTL8720D

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

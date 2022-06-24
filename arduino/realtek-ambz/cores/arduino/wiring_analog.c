/*
 Copyright (c) 2011 Arduino.  All right reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <Arduino.h>
#include <analogin_api.h>
#include <analogout_api.h>
#include <gpio_ex_api.h>
#include <pwmout_api.h>

/* ADC */
analogin_t adc1;
analogin_t adc2;
analogin_t adc3;

bool g_adc_enabled[] = {false, false, false};
// from realtek_amebaz_va0_example/example_sources/adc_vbat/src/main.c
#define AD2MV(ad, offset, gain) (((ad >> 4) - offset) * 1000 / gain)

extern void *gpio_pin_struct[];

extern void pinRemoveMode(pin_size_t pinNumber);

// TODO implement custom ADC calibration

uint16_t analogReadVoltage(pin_size_t pinNumber) {
	uint16_t ret = 0;
	switch (pinNumber) {
#ifdef PIN_A1
		case PIN_A1:
			if (g_adc_enabled[1] == false) {
				analogin_init(&adc2, AD_2);
				g_adc_enabled[1] = true;
			}
			ret = analogin_read_u16(&adc2);
			// AD_1 - 0.0V-5.0V
			return AD2MV(ret, 0x496, 0xBA);
#endif
#ifdef PIN_A0
		case PIN_A0:
			if (g_adc_enabled[0] == false) {
				analogin_init(&adc1, AD_1);
				g_adc_enabled[0] = true;
			}
			ret = analogin_read_u16(&adc1);
			break;
#endif
#ifdef PIN_A2
		case PIN_A2:
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
	PinInfo *pin = pinInfo(pinNumber);
	if (!pin)
		return;
	pwmout_t *obj;

	if (pinSupported(pin, PIN_PWM)) {
		float percent = value * 1.0 / (1 << _analogWriteResolution);
		if (pin->enabled != PIN_PWM) {
			if ((pin->enabled == PIN_GPIO) || (pin->enabled == PIN_IRQ)) {
				pinRemoveMode(pinNumber);
			}
			gpio_pin_struct[pinNumber] = malloc(sizeof(pwmout_t));
			pwmout_t *obj			   = (pwmout_t *)gpio_pin_struct[pinNumber];
			pwmout_init(obj, pin->gpio);
			pwmout_period_us(obj, _analogWritePeriod);
			pwmout_write(obj, percent);
			pin->enabled = PIN_PWM;
		} else {
			pwmout_t *obj = (pwmout_t *)gpio_pin_struct[pinNumber];
			// pwmout_period_us(obj, _writePeriod);
			pwmout_write(obj, percent);
		}
	}
}

typedef struct _tone_argument {
	uint32_t ulPin;
	uint32_t timer_id;
};

void _tone_timer_handler(const void *argument) {
	struct _tone_argument *arg = (struct _tone_argument *)argument;

	uint32_t ulPin = (uint32_t)argument;

	noTone(arg->ulPin);

	os_timer_delete(arg->timer_id);

	free((struct _tone_argument *)arg);
}

void _tone(uint32_t ulPin, unsigned int frequency, unsigned long duration) {
	pwmout_t *obj;

	if ((pinTable[ulPin].supported & PIN_PWM) != PIN_PWM) {
		return;
	}

	if (pinTable[ulPin].enabled != PIN_PWM) {
		if ((pinTable[ulPin].enabled == PIN_GPIO) || (pinTable[ulPin].enabled == PIN_IRQ)) {
			pinRemoveMode(ulPin);
		}
		gpio_pin_struct[ulPin] = malloc(sizeof(pwmout_t));
		pwmout_t *obj		   = (pwmout_t *)gpio_pin_struct[ulPin];
		pwmout_init(obj, pinTable[ulPin].gpio);
		pwmout_period(obj, 1.0 / frequency);
		pwmout_pulsewidth(obj, 1.0 / (frequency * 2));
		pinTable[ulPin].enabled = PIN_PWM;

	} else {
		// There is already a PWM configured
		pwmout_t *obj = (pwmout_t *)gpio_pin_struct[ulPin];
		pwmout_period(obj, 1.0 / frequency);
		pwmout_pulsewidth(obj, 1.0 / (frequency * 2));
	}

	if (duration > 0) {
		struct _tone_argument *arg = (struct _tone_argument *)malloc(sizeof(struct _tone_argument));
		arg->ulPin				   = ulPin;
		arg->timer_id			   = os_timer_create(_tone_timer_handler, 0, arg);
		os_timer_start(arg->timer_id, duration);
	}
}

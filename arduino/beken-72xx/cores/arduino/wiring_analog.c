/* Copyright (c) Kuba Szczodrzyński 2022-06-20. */

#include <Arduino.h>
#include <pwm_pub.h>

static GPIO_INDEX pwmToGpio[] = {
	GPIO6,	// PWM0
	GPIO7,	// PWM1
	GPIO8,	// PWM2
	GPIO9,	// PWM3
	GPIO24, // PWM4
	GPIO26, // PWM5
};

static uint8_t gpioToPwm(GPIO_INDEX gpio) {
	for (uint8_t i = 0; i < sizeof(pwmToGpio); i++) {
		if (pwmToGpio[i] == gpio)
			return i;
	}
	return 0;
}

static pwm_param_t pwm;

uint16_t analogReadVoltage(pin_size_t pinNumber) {
	// TODO
	return 0;
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

	float percent	   = value * 1.0 / (1 << _analogWriteResolution);
	uint32_t dutyCycle = percent * _analogWritePeriod * 26 - 1;
	pwm.channel		   = gpioToPwm(pin->gpio);
	pwm.duty_cycle	   = dutyCycle;

	if (!pinEnabled(pin, PIN_PWM)) {
		// enable PWM and set its value
		pwm.cfg.bits.en		= PWM_ENABLE;
		pwm.cfg.bits.int_en = PWM_INT_DIS;
		pwm.cfg.bits.mode	= PWM_PWM_MODE;
		pwm.cfg.bits.clk	= PWM_CLK_26M;
		pwm.end_value		= 26 * _analogWritePeriod - 1;
		pwm.p_Int_Handler	= NULL;
		__wrap_bk_printf_disable();
		sddev_control(PWM_DEV_NAME, CMD_PWM_INIT_PARAM, &pwm);
		__wrap_bk_printf_enable();
		pin->enabled &= ~PIN_GPIO;
		pin->enabled |= PIN_PWM;
	} else if (value == 0) {
		// disable PWM
		pwm.cfg.bits.en = PWM_DISABLE;
		__wrap_bk_printf_disable();
		sddev_control(PWM_DEV_NAME, CMD_PWM_DEINIT_PARAM, &pwm);
		__wrap_bk_printf_enable();
		pin->enabled &= ~PIN_PWM;
	} else {
		// update duty cycle
		sddev_control(PWM_DEV_NAME, CMD_PWM_SET_DUTY_CYCLE, &pwm);
	}
}

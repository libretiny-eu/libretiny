/* wiring_analog.c for silabs-efm32gg11 — Arduino analogWrite() via TIMER PWM.
 *
 * Static pin->(timer, CC, location) map from the EFM32GG11B AF tables. Each PWM
 * pin lazily inits its timer on first analogWrite() (like micros()'s TIMER0).
 * The pure duty/prescaler math lives in lt_pwm.c (host-tested). The three RGB
 * pins are the only PWM targets the JuiceBox variant exposes; adding a pin is
 * one table row.
 *
 * The app must own the LEDs (status LED off via LT_WIFI_STATUS_LED_DEFAULT=0 or
 * ltWifiStatusLedEnable(false)) — see the WiFi status-LED driver.
 */
#include "ArduinoFamily.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_timer.h"
#include "lt_pwm.h"
#include <Arduino.h>

typedef struct {
	uint8_t pin;			   // encoded (port<<4 | index), matches the variant
	TIMER_TypeDef *timer;	   // TIMER2 / WTIMER0 / WTIMER1
	CMU_Clock_TypeDef clock;   // matching CMU clock
	uint8_t cc;				   // capture/compare channel 0..2
	uint8_t loc;			   // ROUTELOC value (AF table)
	uint8_t timer_bits;		   // 16 (TIMER2) or 32 (WTIMER)
} lt_pwm_pin_t;

static const lt_pwm_pin_t pwm_pins[] = {
	{PIN_PB5, WTIMER0, cmuClock_WTIMER0, 2, 6, 32}, // red   (WTIMER0_CC2 LOC6)
	{PIN_PB6, TIMER2, cmuClock_TIMER2, 0, 4, 16},	// green (TIMER2_CC0  LOC4)
	{PIN_PD8, WTIMER1, cmuClock_WTIMER1, 2, 2, 32}, // blue  (WTIMER1_CC2 LOC2)
};
#define LT_PWM_NPINS (sizeof(pwm_pins) / sizeof(pwm_pins[0]))

static bool pwm_inited[LT_PWM_NPINS];

static int pwm_index(pin_size_t pin) {
	for (unsigned i = 0; i < LT_PWM_NPINS; i++)
		if (pwm_pins[i].pin == (uint8_t)pin)
			return (int)i;
	return -1;
}

void analogWrite(pin_size_t pin, int value) {
	int idx = pwm_index(pin);
	if (idx < 0) {
		// Not PWM-capable: emulate with a digital level (reference-family behavior).
		uint32_t half = (_analogWriteResolution > 0) ? (1u << (_analogWriteResolution - 1)) : 1u;
		pinMode(pin, OUTPUT);
		digitalWrite(pin, ((uint32_t)value >= half) ? HIGH : LOW);
		return;
	}
	const lt_pwm_pin_t *p = &pwm_pins[idx];

	CMU_ClockEnable(p->clock, true);
	uint32_t clk	 = CMU_ClockFreqGet(p->clock);
	lt_pwm_cfg_t cfg = lt_pwm_calc(
		clk, (uint32_t)_analogWritePeriod, (uint8_t)_analogWriteResolution, (uint32_t)value, p->timer_bits
	);

	if (!pwm_inited[idx]) {
		GPIO_PinModeSet(pin_port(p->pin), pin_index(p->pin), gpioModePushPull, 0);

		TIMER_InitCC_TypeDef ccInit = TIMER_INITCC_DEFAULT;
		ccInit.mode					= timerCCModePWM;
		TIMER_InitCC(p->timer, p->cc, &ccInit);

		TIMER_TopSet(p->timer, cfg.top);
		TIMER_CompareSet(p->timer, p->cc, cfg.compare); // arm values before routing the pin

		// Route CCx -> pin. CCxLOC fields are 8 bits apart; ROUTEPEN bit = cc.
		p->timer->ROUTELOC0 =
			(p->timer->ROUTELOC0 & ~(0xFFu << (8u * p->cc))) | ((uint32_t)p->loc << (8u * p->cc));
		p->timer->ROUTEPEN |= (1u << p->cc);

		TIMER_Init_TypeDef tInit = TIMER_INIT_DEFAULT;
		tInit.prescale			 = (TIMER_Prescale_TypeDef)__builtin_ctz(cfg.prescale_div);
		TIMER_Init(p->timer, &tInit);

		pwm_inited[idx] = true;
	} else {
		// Running: buffered updates apply on the next overflow (glitch-free).
		TIMER_TopBufSet(p->timer, cfg.top);
		TIMER_CompareBufSet(p->timer, p->cc, cfg.compare);
	}
}

bool ltPwmDetach(pin_size_t pin) {
	int idx = pwm_index(pin);
	if (idx < 0 || !pwm_inited[idx])
		return false;
	const lt_pwm_pin_t *p = &pwm_pins[idx];
	p->timer->ROUTEPEN &= ~(1u << p->cc); // release the pin back to GPIO control
	pwm_inited[idx] = false;
	return true;
}

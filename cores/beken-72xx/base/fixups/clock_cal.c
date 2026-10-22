#include "include.h"

#include "bk_timer_pub.h"
#include "drv_model_pub.h"
#include "fake_clock_pub.h"
#include "mcu_ps_pub.h"
#include "power_save_pub.h"
#include "pwm_pub.h"
#include "sys_rtos.h"

static CAL_TICK_T cal_tick_save;
UINT32 use_cal_net = 0;

UINT32 fclk_cal_endvalue(UINT32 mode) {
	UINT32 value = 1;

	if (PWM_CLK_32K == mode) {
		/* 32 kHz clock */
		value = FCLK_DURATION_MS * 32;
	} else if (PWM_CLK_26M == mode) {
		/* 26 MHz clock */
		value = FCLK_DURATION_MS * 26000;
	}

	return value;
}

#if CFG_USE_TICK_CAL
static UINT32 timer_cal_init(void) {
	UINT32 fclk;

	fclk = BK_TICKS_TO_MS(fclk_get_tick());

	cal_tick_save.fclk_tick = fclk;
#if CFG_LOW_VOLTAGE_PS && (CFG_SOC_NAME == SOC_BK7252N)
	cal_tick_save.time_us = rtc_reg_get_time_us();
#elif CFG_LOW_VOLTAGE_PS
	cal_tick_save.time_us = cal_get_time_us();
#else
	cal_tick_save.tmp1 = 0;
#endif
	return 0;
}

extern int increase_tick;

static UINT32 timer_cal_tick(void) {
	UINT32 machw = 0;
	INT32 lost;

	GLOBAL_INT_DECLARATION();
	GLOBAL_INT_DISABLE();

#if CFG_LOW_VOLTAGE_PS
	UINT64 delta_fclk = fclk_get_tick() - cal_tick_save.fclk_tick;
#if (CFG_SOC_NAME == SOC_BK7252N)
	UINT64 delta_time = rtc_reg_get_time_us() - cal_tick_save.time_us;
#else
	UINT64 delta_time = cal_get_time_us() - cal_tick_save.time_us;
#endif
	lost = (INT32)(delta_time / 1000 - BK_TICKS_TO_MS(delta_fclk));
#else
	UINT32 fclk = BK_TICKS_TO_MS(fclk_get_tick());
	cal_tick_save.tmp1 += ONE_CAL_TIME;
	UINT32 tmp2 = fclk;
	lost		= (INT32)(cal_tick_save.tmp1 - (UINT32)tmp2);
#endif

	if ((lost >= (2 * FCLK_DURATION_MS))) {
		lost -= FCLK_DURATION_MS;
		fclk_update_tick(BK_MS_TO_TICKS(lost));
#if !CFG_LOW_VOLTAGE_PS
		increase_tick = 0;
#endif
	} else {
		if (lost <= (-(2 * FCLK_DURATION_MS))) {
			if (lost < (-50000)) {
				os_printf("m reset:%x %x\r\n", lost, machw);
			}
#if !CFG_LOW_VOLTAGE_PS
			increase_tick = lost + FCLK_DURATION_MS;
#endif
		}
	}

#if !CFG_LOW_VOLTAGE_PS
	mcu_ps_machw_init();
#endif
	GLOBAL_INT_RESTORE();
	return 0;
}

static void cal_timer_hdl(UINT8 param) {
	timer_cal_tick();
}

static void cal_timer_set(void) {
	timer_param_t param;
	UINT32 ret;
	UINT32 timer_channel;

	timer_cal_init();

	param.channel		= CAL_TIMER_ID;
	param.div			= 1;
	param.period		= ONE_CAL_TIME;
	param.t_Int_Handler = cal_timer_hdl;

	ret = sddev_control(TIMER_DEV_NAME, CMD_TIMER_INIT_PARAM, &param);
	ASSERT(BK_TIMER_SUCCESS == ret);
	timer_channel = param.channel;
	ret			  = sddev_control(TIMER_DEV_NAME, CMD_TIMER_UNIT_ENABLE, &timer_channel);
	ASSERT(BK_TIMER_SUCCESS == ret);
}

static void cal_timer_deset(void) {
	UINT32 ret;
	UINT32 timer_channel;

	timer_channel = CAL_TIMER_ID;
	ret			  = sddev_control(TIMER_DEV_NAME, CMD_TIMER_UNIT_DISABLE, &timer_channel);
	ASSERT(BK_TIMER_SUCCESS == ret);
	timer_cal_init();
}

void fclk_cal_tick(void) {
	timer_cal_tick();
}

UINT32 bk_cal_init(UINT32 setting) {
	GLOBAL_INT_DECLARATION();
	GLOBAL_INT_DISABLE();

	if (1 == setting) {
		use_cal_net = 1;
#if !CFG_LOW_VOLTAGE_PS
		cal_timer_deset();
		mcu_ps_machw_init();
#else
		timer_cal_init();
#endif
		os_printf("decset:%d %d %d %d\r\n", use_cal_net, fclk_get_tick(), fclk_get_second(), xTaskGetTickCount());
	} else {
		use_cal_net = 0;
#if !CFG_LOW_VOLTAGE_PS
		mcu_ps_machw_cal();
		cal_timer_set();
		mcu_ps_machw_reset();
#else
		timer_cal_init();
#endif
		os_printf("cset:%d %d %d %d\r\n", use_cal_net, fclk_get_tick(), fclk_get_second(), xTaskGetTickCount());
	}
	GLOBAL_INT_RESTORE();

	return 0;
}
#endif

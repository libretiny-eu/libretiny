#include "include.h"

#include "bk_timer_pub.h"
#include "drv_model_pub.h"
#include "fake_clock_pub.h"
#include "icu_pub.h"
#include "mcu_ps_pub.h"
#include "power_save_pub.h"
#include "pwm_pub.h"
#include "rtos_pub.h"
#include "sys_rtos.h"
#include "uart_pub.h"

// from LibreTuyaConfig.h
#ifndef LT_MICROS_HIGH_RES
#define LT_MICROS_HIGH_RES 1
#endif

// main FreeRTOS timer ID
static BK_HW_TIMER_INDEX fclk_id = BK_PWM_TIMER_ID0;

extern UINT32 bk_cal_init(UINT32 setting);
extern void mcu_ps_increase_clr(void);
extern uint32_t preempt_delayed_schedule_get_flag(void);
extern void preempt_delayed_schedule_clear_flag(void);

// forward definitions
static void fclk_timer_hw_init(BK_HW_TIMER_INDEX timer_id);
static void fclk_hdl(UINT8 param);

void fclk_init(void) {
#if (CFG_SOC_NAME == SOC_BK7231)
	fclk_timer_hw_init(BK_PWM_TIMER_ID0);
#elif LT_MICROS_HIGH_RES
	fclk_timer_hw_init(BK_TIMER_ID0);
#else
	fclk_timer_hw_init(BK_TIMER_ID3);
#endif

#if CFG_USE_TICK_CAL
	bk_cal_init(0);
#endif
}

/* timer_id: BK_PWM_TIMER_ID0 or BK_TIMER_ID3 */
static void fclk_timer_hw_init(BK_HW_TIMER_INDEX timer_id) {
#if (CFG_SOC_NAME == SOC_BK7231)
	ASSERT(timer_id >= BK_PWM_TIMER_ID0);
#endif

	fclk_id = timer_id;
	if (fclk_id >= BK_PWM_TIMER_ID0) { // pwm timer
		pwm_param_t param;
		param.channel		  = (fclk_id - PWM0);
		param.cfg.bits.en	  = PWM_ENABLE;
		param.cfg.bits.int_en = PWM_INT_EN;
		param.cfg.bits.mode	  = PWM_TIMER_MODE;
		param.cfg.bits.clk	  = PWM_CLK_26M;
		param.p_Int_Handler	  = fclk_hdl;
#if (CFG_SOC_NAME == SOC_BK7231N)
		param.duty_cycle1 = 0;
#else
		param.duty_cycle = 0;
#endif
		param.end_value = fclk_cal_endvalue((UINT32)param.cfg.bits.clk);

		sddev_control(PWM_DEV_NAME, CMD_PWM_INIT_PARAM, &param);
	} else { // timer
		timer_param_t param;
		param.channel = fclk_id;
		param.div	  = 1;
#if LT_MICROS_HIGH_RES
		param.period = FCLK_DURATION_MS * 1000;
#else
		param.period	 = FCLK_DURATION_MS;
#endif
		param.t_Int_Handler = fclk_hdl;

#if LT_MICROS_HIGH_RES
		UINT32 ret = sddev_control(TIMER_DEV_NAME, CMD_TIMER_INIT_PARAM_US, &param);
#else
		UINT32 ret		 = sddev_control(TIMER_DEV_NAME, CMD_TIMER_INIT_PARAM, &param);
#endif
		ASSERT(BK_TIMER_SUCCESS == ret);
		sddev_control(TIMER_DEV_NAME, CMD_TIMER_UNIT_ENABLE, &param.channel);
	}
}

static void fclk_hdl(UINT8 param) {
#if CFG_USE_TICK_CAL
	if (!mcu_ps_need_pstick())
		return;
#endif
	GLOBAL_INT_DECLARATION();
	GLOBAL_INT_DISABLE();
	if (xTaskIncrementTick() != pdFALSE || preempt_delayed_schedule_get_flag()) {
		preempt_delayed_schedule_clear_flag();
		/* Select a new task to run. */
		vTaskSwitchContext();
	}
	GLOBAL_INT_RESTORE();
}

UINT32 fclk_update_tick(UINT32 tick) {
	GLOBAL_INT_DECLARATION();
	if (tick == 0)
		return 0;
	GLOBAL_INT_DISABLE();
	mcu_ps_increase_clr();
	vTaskStepTick(tick);
	GLOBAL_INT_RESTORE();
	return 0;
}

UINT64 fclk_get_tick(void) {
	return xTaskGetTickCount();
}

UINT32 fclk_get_second(void) {
	return xTaskGetTickCount() / FCLK_SECOND;
}

BK_HW_TIMER_INDEX fclk_get_tick_id(void) {
	return fclk_id;
}

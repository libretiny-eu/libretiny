#include "StupidServoController.hpp"

#include "Bsp.hpp"
#include "ControlLink.hpp"
#include "stm32f1xx_hal_rcc.h"
#include "stm32f1xx_ll_tim.h"

static uint32_t pwmCenterValue;
static float pwmCoef;

void stupidServoInit() {
    LL_TIM_InitTypeDef pwmInit;
    LL_TIM_StructInit(&pwmInit);
    auto apb1TimClk = 2 * HAL_RCC_GetPCLK1Freq();

    // 1/50 s :
    pwmInit.Autoreload = 65535;
    pwmInit.Prescaler = (apb1TimClk / 50) / pwmInit.Autoreload;
    pwmInit.CounterMode = LL_TIM_COUNTERMODE_UP;
    pwmInit.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    pwmInit.RepetitionCounter = 0;

    // Center at 1.5ms within 1/50s
    pwmCenterValue = uint32_t(pwmInit.Autoreload * 0.075f);
    // Range +- 0.5ms within 1/50s
    pwmCoef = pwmInit.Autoreload * 0.025f;

    LL_TIM_OC_InitTypeDef ocInit;
    LL_TIM_OC_StructInit(&ocInit);
    ocInit.OCMode = LL_TIM_OCMODE_PWM1;
    ocInit.OCState = LL_TIM_OCSTATE_DISABLE;
    ocInit.CompareValue = pwmCenterValue;
    ocInit.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
    ocInit.OCIdleState = LL_TIM_OCIDLESTATE_LOW;
    LL_TIM_Init(servoTimer, &pwmInit);

    for (uint16_t channel = LL_TIM_CHANNEL_CH1; channel != 0; channel <<= 4) {
        LL_TIM_OC_Init(servoTimer, channel, &ocInit);
        LL_TIM_OC_EnablePreload(servoTimer, channel);
    }
    LL_TIM_SetOffStates(servoTimer, LL_TIM_OSSI_DISABLE, LL_TIM_OSSR_ENABLE);
    LL_TIM_GenerateEvent_UPDATE(servoTimer);
    LL_TIM_EnableCounter(servoTimer);
    LL_TIM_EnableAllOutputs(servoTimer);
}

static void stupidServoDisable(int servoIndex) {
    switch (servoIndex) {
    case 0:
        LL_TIM_CC_DisableChannel(servoTimer, LL_TIM_CHANNEL_CH1);
        break;
    case 1:
        LL_TIM_CC_DisableChannel(servoTimer, LL_TIM_CHANNEL_CH2);
        break;
    case 2:
        LL_TIM_CC_DisableChannel(servoTimer, LL_TIM_CHANNEL_CH3);
        break;
    case 3:
        LL_TIM_CC_DisableChannel(servoTimer, LL_TIM_CHANNEL_CH4);
        break;
    }
}

static void stupidServoSetPosition(int servoIndex, float position) {
    uint32_t value = pwmCenterValue + int32_t(pwmCoef * position);

    switch (servoIndex) {
    case 0:
        LL_TIM_CC_EnableChannel(servoTimer, LL_TIM_CHANNEL_CH1);
        LL_TIM_OC_SetCompareCH1(servoTimer, value);
        break;
    case 1:
        LL_TIM_CC_EnableChannel(servoTimer, LL_TIM_CHANNEL_CH2);
        LL_TIM_OC_SetCompareCH2(servoTimer, value);
        break;
    case 2:
        LL_TIM_CC_EnableChannel(servoTimer, LL_TIM_CHANNEL_CH3);
        LL_TIM_OC_SetCompareCH3(servoTimer, value);
        break;
    case 3:
        LL_TIM_CC_EnableChannel(servoTimer, LL_TIM_CHANNEL_CH4);
        LL_TIM_OC_SetCompareCH4(servoTimer, value);
        break;
    }
}

void stupidServoDispatch(const CoprocReq_SetStupidServo& request) {
    switch (request.which_servoCmd) {
    case CoprocReq_SetStupidServo_disable_tag:
        stupidServoDisable(request.servoIndex);
        break;
    case CoprocReq_SetStupidServo_setPosition_tag:
        stupidServoSetPosition(
            request.servoIndex, request.servoCmd.setPosition);
        break;
    }

    CoprocStat status;
    status.which_payload = CoprocStat_stupidServoStat_tag;
    controlLinkTx(status);
}

void stupidServoReset() {
    for (int i = 0; i < 4; ++i) {
        stupidServoDisable(i);
    }
}

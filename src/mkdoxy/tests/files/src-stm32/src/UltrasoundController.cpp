#include "UltrasoundController.hpp"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_rcc.h"
#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_tim.h"
#include "utils/Debug.hpp"
#include "utils/QueueWrapper.hpp"
#include "utils/TaskWrapper.hpp"

#include "Bsp.hpp"
#include "Dispatcher.hpp"

static const uint16_t pingTimeoutMs = 30;
static const uint16_t echoRestoreTimeoutMs = 180;
static CoprocStat status;
static int utsActiveIndex;
static uint32_t risingMicros;
static TaskWrapper<512> utsTask;
static QueueWrapper<int, 16> trigQueue;

void ultrasoundInit() {
    LL_TIM_InitTypeDef timInit;
    LL_TIM_StructInit(&timInit);
    auto apb1TimClk = 2 * HAL_RCC_GetPCLK1Freq();

    timInit.Autoreload = pingTimeoutMs * 1000;
    timInit.Prescaler = apb1TimClk / 1'000'000; // 1 us ticks
    LL_TIM_Init(utsTimer, &timInit);
    LL_TIM_SetOnePulseMode(utsTimer, LL_TIM_ONEPULSEMODE_SINGLE);
    LL_TIM_SetUpdateSource(utsTimer, LL_TIM_UPDATESOURCE_COUNTER);
    LL_TIM_ClearFlag_UPDATE(utsTimer);

    for (const auto& p : utsEchoPin) {
        LL_EXTI_DisableIT_0_31(p.second);
    }

    trigQueue.create();
    utsTask.start("ultrasound", ultrasoundTaskPrio, []() {
        while (true) {
            int utsIndex;
            trigQueue.pop_front(utsIndex);

            // Wait for potential ECHO high
            auto echoWait = [=]() {
                TickType_t before = xTaskGetTickCount();
                while (pinRead(utsEchoPin[utsIndex])) {
                    if ((xTaskGetTickCount() - before)
                        > pdMS_TO_TICKS(echoRestoreTimeoutMs)) {
                        return false;
                    }
                    vTaskDelay(pdMS_TO_TICKS(5));
                }
                return true;
            };
            if (!echoWait()) {
                DEBUG("Ultrasound ECHO %d hanging high\n", utsIndex);
                continue;
            }

            // Reset timer
            LL_TIM_GenerateEvent_UPDATE(utsTimer);
            LL_TIM_ClearFlag_UPDATE(utsTimer);

            // Perform TRIG pulse
            taskENTER_CRITICAL();
            LL_TIM_EnableCounter(utsTimer);
            pinWrite(utsTrigPin[utsIndex], true);
            while (LL_TIM_GetCounter(utsTimer) < 10
                && !LL_TIM_IsActiveFlag_UPDATE(utsTimer)) { // 10 us pulse
            }
            pinWrite(utsTrigPin[utsIndex], false);
            taskEXIT_CRITICAL();

            // Wait for ECHO measurement from ISR
            utsActiveIndex = utsIndex;
            xTaskNotifyStateClear(nullptr);
            LL_EXTI_EnableIT_0_31(utsEchoPin[utsIndex].second);
            uint32_t micros = 0;
            auto ok = xTaskNotifyWait(
                0U, ~0U, &micros, pdMS_TO_TICKS(pingTimeoutMs));
            LL_EXTI_DisableIT_0_31(utsEchoPin[utsIndex].second);

            // Send measurement
            status = CoprocStat_init_default;
            status.which_payload = CoprocStat_ultrasoundStat_tag;
            status.payload.ultrasoundStat.utsIndex = utsIndex;
            status.payload.ultrasoundStat.roundtripMicrosecs = ok ? micros : 0;
            dispatcherEnqueueStatus(status);
        }
    });
}

void ultrasoundDispatch(const CoprocReq_UltrasoundReq& request) {
    if (request.utsIndex > 3) {
        DEBUG("Ultrasound index %lu out of range\n", request.utsIndex);
        return;
    }

    switch (request.which_utsCmd) {
    case CoprocReq_UltrasoundReq_singlePing_tag:
        if (!trigQueue.push_back(request.utsIndex, 0)) {
            DEBUG("Ultrasound TRIG queue overflow\n");
        }
        break;
    }
}

void ultrasoundOnEchoEdge() {
    auto nowMicros = LL_TIM_GetCounter(utsTimer);

    if (pinRead(utsEchoPin[utsActiveIndex])) {
        risingMicros = nowMicros;
    } else if (risingMicros > 0) {
        uint32_t resultMicros = nowMicros - risingMicros;
        if (nowMicros < risingMicros) {
            resultMicros = 0;
        }

        BaseType_t woken = 0;
        xTaskNotifyFromISR(
            utsTask.handle(), resultMicros, eSetValueWithoutOverwrite, &woken);
        portYIELD_FROM_ISR(woken);
    }
}

void ultrasoundReset() { trigQueue.reset(); }

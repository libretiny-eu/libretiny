#include "Bsp.hpp"
#include "BuzzerController.hpp"
#include "ControlLink.hpp"
#include "Dispatcher.hpp"
#include "Esp32Manager.hpp"
#include "I2cController.hpp"
#include "MotorController.hpp"
#include "MpuController.hpp"
#include "StupidServoController.hpp"
#include "UltrasoundController.hpp"
#include "utils/Debug.hpp"
#include "utils/TaskWrapper.hpp"

static TaskWrapper<1024> softResetTask;

extern "C" void EXTI4_IRQHandler(void) {
    // This EXTI vector only serves ESP_EN.
    sEsp32Manager.onEnRisingInIrq();
    __HAL_GPIO_EXTI_CLEAR_IT(espEnPin.second);
}

extern "C" void EXTI9_5_IRQHandler(void) {
    // This EXTI vector only serves the ultrasound ECHO
    // of which only one is listening at a time.
    // We want minimum jitter so avoid the stupid HAL callback scheme.

    ultrasoundOnEchoEdge();
    __HAL_GPIO_EXTI_CLEAR_IT(uts1EchoPin.second | uts2EchoPin.second
        | uts3EchoPin.second | uts4EchoPin.second);
}

void softResetInit() {
    softResetTask.start("softrst", softResetTaskPrio, []() {
        while (true) {
            if (xTaskNotifyWait(0, 0, nullptr, portMAX_DELAY) != pdTRUE)
                continue;

            DEBUG("Soft resetting peripherials to default state.\n");

            setLeds(0);
            buzzerSetState(false);
            dispatcherReset();
            controlLinkReset();
            stupidServoReset();
            ultrasoundReset();
            motorReset();
            i2cReset();
            mpuReset();
        }
    });
}

void softReset() {
    if (isInInterrupt()) {
        BaseType_t woken = pdFALSE;
        xTaskNotifyFromISR(softResetTask.handle(), 0, eNoAction, &woken);
        portYIELD_FROM_ISR(woken);
    } else {
        xTaskNotify(softResetTask.handle(), 0, eNoAction);
    }
}

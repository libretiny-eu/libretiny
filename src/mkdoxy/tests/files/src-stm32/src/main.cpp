#include <array>

#include "stm32f1xx_hal.h"

#include "utils/Debug.hpp"
#include "utils/TaskWrapper.hpp"

#include "ButtonController.hpp"
#include "I2cController.hpp"
#include "MotorController.hpp"
#include "MpuController.hpp"
#include "OledController.hpp"
#include "StupidServoController.hpp"
#include "UltrasoundController.hpp"

#include "Bsp.hpp"
#include "CdcUartTunnel.hpp"
#include "ControlLink.hpp"
#include "DebugLink.hpp"
#include "Dispatcher.hpp"
#include "Esp32Manager.hpp"
#include "Power.hpp"
#include "UsbCdcLink.h"

static TaskWrapper<3072> mainTask;

typedef int _myint;

_myint testInt() {}

int main() {
    clocksInit();
    HAL_Init();

#ifdef RBCX_VECT_TAB_OFFSET
    SCB->VTOR = FLASH_BASE | RBCX_VECT_TAB_OFFSET;
#endif

    pinsInit();

    mainTask.start("main", mainTaskPrio, []() {
        debugUartInit();
        softResetInit();
        powerInit();
        dispatcherInit();
        tunnelUartInit();
        controlUartInit();
        cdcLinkInit();
        I2Cdev_init();
        stupidServoInit();
        ultrasoundInit();
        mpuCreate();
        sEsp32Manager.init();
        motorInit();

        DEBUG("STM32 Coprocessor initialized, v%06x " RBCX_VER_REVISION
                  RBCX_VER_DIRTY_STR "\n",
            RBCX_VER_NUMBER);
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(1));
            debugLinkPoll();
            powerPoll();
            dispatcherPoll();
            tunnelPoll();
            buttonControllerPoll();
            sEsp32Manager.poll();
        }
    });

    vTaskStartScheduler();
    abort();
}
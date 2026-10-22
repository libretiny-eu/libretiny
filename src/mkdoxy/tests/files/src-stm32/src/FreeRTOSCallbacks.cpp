#include "stm32f1xx_hal.h"
#include "stm32f1xx_ll_utils.h"

#include "Bsp.hpp"
#include "FreeRTOS.h"
#include "task.h"

static StaticTask_t idleTaskStruct;
static StackType_t idleTaskStack[configMINIMAL_STACK_SIZE];

// While timeouts in LL functions rely on having milliseconds in SYSTICK,
// timeouts in HAL functions require incrementing the uwTick variable:
extern "C" void vApplicationTickHook() { HAL_IncTick(); }

extern "C" void vApplicationGetIdleTaskMemory(
    StaticTask_t** ppxIdleTaskTCBBuffer, StackType_t** ppxIdleTaskStackBuffer,
    uint32_t* pulIdleTaskStackSize) {
    *ppxIdleTaskTCBBuffer = &idleTaskStruct;
    *ppxIdleTaskStackBuffer = idleTaskStack;
    *pulIdleTaskStackSize = sizeof(idleTaskStack) / sizeof(StackType_t);
}

#if configUSE_TIMERS
static StaticTask_t timerTaskStruct;
static StackType_t timerTaskStack[configTIMER_TASK_STACK_DEPTH];

extern "C" void vApplicationGetTimerTaskMemory(
    StaticTask_t** ppxIdleTaskTCBBuffer, StackType_t** ppxIdleTaskStackBuffer,
    uint32_t* pulIdleTaskStackSize) {
    *ppxIdleTaskTCBBuffer = &timerTaskStruct;
    *ppxIdleTaskStackBuffer = timerTaskStack;
    *pulIdleTaskStackSize = sizeof(timerTaskStack) / sizeof(StackType_t);
}
#endif

extern "C" void vApplicationStackOverflowHook(
    TaskHandle_t xTask, signed char* pcTaskName) {
    printf("Stack overflow!\n");
    printf("Task: %s\n", pcTaskName);

    abort();
}

extern "C" void _exit() {
    softReset();

    LL_mDelay(10);
    puts("\n\n !!! PROGRAM EXITED, HALTING !!!\n\n");
    LL_mDelay(10);

    __disable_irq();

    uint32_t on = 0xFFFFFFF;
    while (true) {
        setLeds(on);
        on = ~on;

        // Triggers when STLink is connected, there doesn't seem to be a way
        // to check if a debugging session is in progress :/
        if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) {
            __BKPT();
        }

        LL_mDelay(200);
    }
}

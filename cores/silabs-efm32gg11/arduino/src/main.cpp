/* Phase 1 main.cpp for silabs-efm32gg11.
 *
 * Pattern (matches other LibreTiny families):
 *   - cores/common/arduino/src/main.c defines `int main(void)`; the family does NOT.
 *     Family's job here is to provide `startMainTask()` (FreeRTOS bootstrap).
 *   - lt_init_family() runs before main() via the GSDK startup's `bl __START`
 *     hook redirected to `lt_main` in builder/family/silabs-efm32gg11.py
 *     (-D__START=lt_main). lt_main() then calls
 *     lt_init_family -> lt_init_variant -> __libc_init_array -> fal_init -> main().
 *     So `startMainTask` does NOT need to call lt_init_family itself.
 */

#include <ArduinoPrivate.h>

extern "C" {

#include "FreeRTOS.h"
#include "lt_family.h"
#include "task.h"

#ifndef LT_MAIN_TASK_STACK_SIZE
#define LT_MAIN_TASK_STACK_SIZE (4096)
#endif

#ifndef LT_MAIN_TASK_PRIORITY
#define LT_MAIN_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#endif

bool startMainTask(void) {
	BaseType_t rc = xTaskCreate(
		(TaskFunction_t)mainTask,
		"main",
		LT_MAIN_TASK_STACK_SIZE / sizeof(StackType_t),
		NULL,
		LT_MAIN_TASK_PRIORITY,
		NULL
	);
	if (rc != pdPASS) {
		return false;
	}

	vTaskStartScheduler();
	// Should not reach here.
	return true;
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *name) {
	(void)task;
	printf("*** stack overflow in task '%s' ***\r\n", name);
	NVIC_SystemReset();
}

void vApplicationMallocFailedHook(void) {
	printf("*** FreeRTOS malloc failed *** task='%s' free=%u\r\n",
	       pcTaskGetName(NULL), (unsigned) xPortGetFreeHeapSize());
	NVIC_SystemReset();
}

} // extern "C"

/* Copyright (c) WGM160P-LibreTiny port 2026-05-26.
 *
 * Phase 1 FreeRTOSConfig for silabs-efm32gg11 — minimum surface for blink +
 * Serial + 1 user task. Tickless idle and dynamic priority changes deliberately
 * off to keep bring-up failure surface small.
 */

#pragma once

#include <stdint.h>
extern uint32_t SystemCoreClock;

#define configUSE_PREEMPTION					1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TICKLESS_IDLE					0
#define configCPU_CLOCK_HZ						(SystemCoreClock)
#define configTICK_RATE_HZ						(1000)
#define configMAX_PRIORITIES					(7)
#define configMINIMAL_STACK_SIZE				(128)
#define configTOTAL_HEAP_SIZE					(64 * 1024)
#define configMAX_TASK_NAME_LEN					(16)
#define configUSE_16_BIT_TICKS					0
#define configIDLE_SHOULD_YIELD					1
#define configUSE_TASK_NOTIFICATIONS			1
#define configUSE_MUTEXES						1
#define configUSE_RECURSIVE_MUTEXES				1
#define configUSE_COUNTING_SEMAPHORES			1
#define configUSE_QUEUE_SETS					0
#define configQUEUE_REGISTRY_SIZE				10
#define configUSE_TIME_SLICING					1
#define configUSE_NEWLIB_REENTRANT				0
#define configENABLE_BACKWARD_COMPATIBILITY		0
#define configRECORD_STACK_HIGH_ADDRESS			1

/* Memory allocation */
#define configSUPPORT_STATIC_ALLOCATION	 0
#define configSUPPORT_DYNAMIC_ALLOCATION 1

/* Hook functions — the error catchers wired in the spec §4 */
#define configUSE_IDLE_HOOK				   0
#define configUSE_TICK_HOOK				   0
#define configCHECK_FOR_STACK_OVERFLOW	   2
#define configUSE_MALLOC_FAILED_HOOK	   1
#define configUSE_DAEMON_TASK_STARTUP_HOOK 0

/* Run-time and task stats — off in Phase 1 */
#define configGENERATE_RUN_TIME_STATS		 0
#define configUSE_TRACE_FACILITY			 0
#define configUSE_STATS_FORMATTING_FUNCTIONS 0

/* Co-routines (deprecated, off) */
#define configUSE_CO_ROUTINES			0
#define configMAX_CO_ROUTINE_PRIORITIES 1

/* Software timers (off in Phase 1) */
#define configUSE_TIMERS 0

/* Optional API features */
#define INCLUDE_vTaskPrioritySet			1
#define INCLUDE_uxTaskPriorityGet			1
#define INCLUDE_vTaskDelete					1
#define INCLUDE_vTaskSuspend				1
#define INCLUDE_vTaskDelayUntil				1
#define INCLUDE_vTaskDelay					1
#define INCLUDE_xTaskGetSchedulerState		1
#define INCLUDE_xTaskGetCurrentTaskHandle	1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_xTaskGetIdleTaskHandle		0
#define INCLUDE_eTaskGetState				1
#define INCLUDE_xEventGroupSetBitFromISR	1
#define INCLUDE_xTimerPendFunctionCall		0
#define INCLUDE_xTaskAbortDelay				0
#define INCLUDE_xTaskGetHandle				1
#define INCLUDE_xTaskResumeFromISR			1

/* Cortex-M4 interrupt priority knobs */
#define configPRIO_BITS								 3 /* EFM32GG11 NVIC: 8 priority levels */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY		 7
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configKERNEL_INTERRUPT_PRIORITY				 (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY		 (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* Map FreeRTOS handler names to CMSIS vector names */
#define vPortSVCHandler		SVC_Handler
#define xPortPendSVHandler	PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#define configASSERT(x)           \
	if ((x) == 0) {               \
		taskDISABLE_INTERRUPTS(); \
		for (;;)                  \
			;                     \
	}

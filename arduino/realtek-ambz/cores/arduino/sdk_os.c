#include "sdk_os.h"

#include <Arduino.h>
#include <cmsis_os.h>

uint32_t os_thread_create(void (*task)(const void *argument), void *argument, int priority, uint32_t stack_size) {

	osThreadDef_t thread_def;

	thread_def.pthread	 = task;
	thread_def.tpriority = (osPriority)priority;
	// the underlying freertos implementation on cmsis os divide stack size by 4
	thread_def.stacksize = stack_size * 4;
	thread_def.name		 = "ARDUINO";

	return (uint32_t)osThreadCreate(&thread_def, argument);
}

uint32_t os_thread_get_id(void) {
	return osThreadGetId();
}

uint32_t os_thread_terminate(uint32_t thread_id) {
	return (uint32_t)osThreadTerminate(thread_id);
}

uint32_t os_thread_yield(void) {
	return (uint32_t)osThreadYield();
}

uint32_t os_thread_set_priority(uint32_t thread_id, int priority) {
	return (uint32_t)osThreadSetPriority(thread_id, (osPriority)priority);
}

int os_thread_get_priority(uint32_t thread_id) {
	return (int)osThreadGetPriority(thread_id);
}

int32_t os_signal_set(uint32_t thread_id, int32_t signals) {
	return osSignalSet(thread_id, signals);
}

int32_t os_signal_clear(uint32_t thread_id, int32_t signals) {
	return osSignalClear(thread_id, signals);
}

os_event_t os_signal_wait(int32_t signals, uint32_t millisec) {

	osEvent evt;
	os_event_t ret;

	evt				   = osSignalWait(signals, millisec);
	ret.status		   = (uint32_t)evt.status;
	ret.value.signals  = evt.value.signals;
	ret.def.message_id = evt.def.message_id;

	return ret;
}

typedef void (*os_ptimer)(const void *argument);

uint32_t os_timer_create(void (*callback)(const void *argument), uint8_t isPeriodic, void *argument) {

	osTimerDef_t *pTimerDef;

	pTimerDef		  = (osTimerDef_t *)malloc(sizeof(osTimerDef_t));
	pTimerDef->ptimer = callback;
	pTimerDef->custom = (struct os_timer_custom *)malloc(sizeof(struct os_timer_custom));

	return osTimerCreate(pTimerDef, (isPeriodic ? osTimerPeriodic : osTimerOnce), argument);
}

uint32_t os_timer_start(uint32_t timer_id, uint32_t millisec) {
	return osTimerStart(timer_id, millisec);
}

uint32_t os_timer_stop(uint32_t timer_id) {
	return osTimerStop(timer_id);
}

uint32_t os_timer_delete(uint32_t timer_id) {

	osTimerDef_t *pTimerDef;

	pTimerDef = (osTimerDef_t *)pvTimerGetTimerID(timer_id);
	free(pTimerDef->custom);
	free(pTimerDef);

	return osTimerDelete(timer_id);
}

uint32_t os_semaphore_create(int32_t count) {
	return (uint32_t)osSemaphoreCreate(NULL, count);
}

int32_t os_semaphore_wait(uint32_t semaphore_id, uint32_t millisec) {
	if (osSemaphoreWait((osSemaphoreId)semaphore_id, millisec) == 0) {
		return 1;
	} else {
		return 0;
	}
}

uint32_t os_semaphore_release(uint32_t semaphore_id) {
	return (uint32_t)osSemaphoreRelease((osSemaphoreId)semaphore_id);
}

uint32_t os_semaphore_delete(uint32_t semaphore_id) {
	return (uint32_t)osSemaphoreDelete((osSemaphoreId)semaphore_id);
}

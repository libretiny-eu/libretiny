#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @defgroup os_status os_status
 * Redefinition from enum osStatus
 * @{
 */
/** function completed; no error or event occurred. */
#define OS_OK					  0x00
/** function completed; signal event occurred. */
#define OS_EVENT_SIGNAL			  0x08
/** function completed; message event occurred. */
#define OS_EVENT_MESSAGE		  0x10
/** function completed; mail event occurred. */
#define OS_EVENT_MAIL			  0x20
/** function completed; timeout occurred. */
#define OS_EVENT_TIMEOUT		  0x40
/** parameter error: a mandatory parameter was missing or specified an incorrect object. */
#define OS_ERROR_PARAMETER		  0x80
/** resource not available: a specified resource was not available. */
#define OS_ERROR_RESOURCE		  0x81
/** resource not available within given time: a specified resource was not available within the timeout period. */
#define OS_ERROR_TIMEOUT_RESOURCE 0xC1
/** not allowed in ISR context: the function cannot be called from interrupt service routines. */
#define OS_ERROR_ISR			  0x82
/** function called multiple times from ISR with same object. */
#define OS_ERROR_ISR_RECURSIVE	  0x83
/** system cannot determine priority or thread has illegal priority. */
#define OS_ERROR_PRIORITY		  0x84
/** system is out of memory: it was impossible to allocate or reserve memory for the operation. */
#define OS_ERROR_NO_MEMORY		  0x85
/** value of a parameter is out of range. */
#define OS_ERROR_VALUE			  0x86
/** unspecified RTOS error: run-time error but no other error message fits. */
#define OS_ERROR_OS				  0xFF
/** @} */ // end of group os_status

/**
 * @defgroup os_priority os_priority
 * Redefinition from enum osPriority
 * @{
 */
/** priority: idle (lowest) */
#define OS_PRIORITY_IDLE		 (-3)
/** priority: low */
#define OS_PRIORITY_LOW			 (-2)
/** priority: below normal */
#define OS_PRIORITY_BELOW_NORMAL (-1)
/** priority: normal (default) */
#define OS_PRIORITY_NORMAL		 (0)
/** priority: above normal */
#define OS_PRIORITY_ABOVENORMAL	 (+1)
/** priority: high */
#define OS_PRIORITY_HIGH		 (+2)
/** priority: realtime (highest) */
#define OS_PRIORITY_REALTIME	 (+3)
/** @} */ // end of group os_priority

#ifndef DEFAULT_STACK_SIZE
/**
 * @ingroup wiring_os
 * @brief default stack size
 *
 * It is suggest that thread is assigned stack size more than DEFAULT_STACK_SIZE
 */
#define DEFAULT_STACK_SIZE 512
#endif

/**
 * @ingroup wiring_os
 * @struct os_event_t
 * Redefine osEvent in cmsis_os.h
 */
typedef struct {
	uint32_t status; ///< status code: event or error information

	union {
		uint32_t v;		 ///< message as 32-bit value
		void *p;		 ///< message or mail as void pointer
		int32_t signals; ///< signal flags
	} value;			 ///< event value

	union {
		void *mail_id;	  ///< mail id obtained by osMailCreate
		void *message_id; ///< message id obtained by osMessageCreate
	} def;				  ///< event definition
} os_event_t;

/**
 * @defgroup thread_management thread_management
 * Thread management include create, get thread id, terminate, yield, and set/get priority
 **/

/**
 * @ingroup thread_management
 * @brief Create a thread and add it to Active Threads and set it to state READY.
 *
 * @param[in] task Function pointer which is the thread body. It should not run into the end of function unless
 * os_thread_terminate is invoked
 * @param[in] argument the data pointer which brings to task
 * @param[in] priority The underlying os is FreeRTOS. It executes tasks with highest priority which are not in idle
 * state.\n If there are more than 2 tasks to be executed, then they share the time slice.
 * @param[in] stack_size The stack_size is used as memory heap only for this task. \n
 *     The local variables and call stacks would occupy this heap. Please make sure the the stack_size is big enough to
 * avoid curroption
 * @return The thread id which is used in thread operation and signaling.
 */
extern uint32_t os_thread_create(void (*task)(const void *argument), void *argument, int priority, uint32_t stack_size);

/**
 * @ingroup thread_management
 * @brief Return the thread ID of the current running thread.
 *
 * @return Current thread id which calls os_thread_get_id
 */
extern uint32_t os_thread_get_id(void);

/**
 * @ingroup thread_management
 * @brief Terminate execution of a thread and remove it from Active Threads.
 *
 * Thread should not ended without terminate first
 *
 * @param[in] thread_id Terminate the thread with specific thread_id
 * @return os_status code
 */
extern uint32_t os_thread_terminate(uint32_t thread_id);

/**
 * @ingroup thread_management
 * @brief Pass control to next thread that is in state \b READY.
 *
 * By default the minimal execution unit is 1 millisecond. In a scenario that if a thread with smaller want to handout
 * execution right to a thread with higher priority immediately without waiting for the ending of current 1 millisecond,
 * then invoke os_thread_yield can transfer exection right to OS's idle task and check which is the next execution
 * thread.
 *
 * @return os_status code
 */
extern uint32_t os_thread_yield(void);

/**
 * @ingroup thread_management
 * @brief Change priority of an active thread.
 *
 * @param[in] thread_id The target thread with the thread id to be changed
 * @param[in] priority The updated priority
 * @return os_status code
 */
extern uint32_t os_thread_set_priority(uint32_t thread_id, int priority);

/**
 * @ingroup thread_management
 * @brief Get current priority of an active thread.
 *
 * @param[in] thread_id The target thread with the thread id to be searched
 * @return os_priority
 */
extern int os_thread_get_priority(uint32_t thread_id);

/**
 * @defgroup signal_management signal_management
 * Signaling between threads include set, clear, and wait
 */

/**
 * @ingroup signal_management
 * @brief Set the specified Signal Flags of an active thread.
 *
 * @param[in] thread_id Send signal to a thread with the thread id
 * @param[in] signals the signals to be send
 * @return os_status code
 */
extern int32_t os_signal_set(uint32_t thread_id, int32_t signals);

/**
 * @ingroup signal_management
 * @brief Clear the specified Signal Flags of an active thread.
 *
 * @param[in] thread_id Clear signal to a thread with the thread id
 * @param[in] signals The signals to be clear
 * @return os_status code
 */
extern int32_t os_signal_clear(uint32_t thread_id, int32_t signals);

/**
 * @ingroup signal_management
 * @brief Wait for one or more Signal Flags to become signaled for the current \b RUNNING thread.
 *
 * @param[in] signals the signals to be wait
 * @param[in] millisec the timeout value if no signal comes in. Fill in 0xFFFFFFFF for infinite wait.
 * @return os_status code
 */
extern os_event_t os_signal_wait(int32_t signals, uint32_t millisec);

/**
 * @defgroup timer_management timer_management
 * Software timer management include create, start, stop, delete.
 */

/**
 * @ingroup timer_management
 * @brief specify timer type that invoke only once
 */
#define OS_TIMER_ONCE (0)

/**
 * @ingroup timer_management
 * @brief specify timer type that invoke periodically
 */
#define OS_TIMER_PERIODIC (1)

/**
 * @ingroup timer_management
 * @brief Create a timer.
 *
 * @param[in] callback The function to be invoke when timer timeout
 * @param[in] isPeriodic \b OS_TIMER_ONCE or \b OS_TIMER_PERIODIC
 * @param[in] argument The argument that is bring into callback function
 * @return timer id
 */
extern uint32_t os_timer_create(void (*callback)(const void *argument), uint8_t isPeriodic, void *argument);

/**
 * @ingroup timer_management
 * @brief Start or restart a timer.
 *
 * @param[in] timer_id The timer id obtained from by os_timer_create
 * @param[in] millisec The delays after timer starts
 * @return os_status code
 */
extern uint32_t os_timer_start(uint32_t timer_id, uint32_t millisec);

/**
 * @ingroup timer_management
 * @brief Stop the timer.
 *
 * @param[in] timer_id The timer id obtained from by os_timer_create
 * @return os_status code
 */
extern uint32_t os_timer_stop(uint32_t timer_id);

/**
 * @ingroup timer_management
 * @brief Delete a timer that was created by os_timer_create
 *
 * @param[in] timer_id The timer id obtained from by os_timer_create
 * @return os_status code
 */
extern uint32_t os_timer_delete(uint32_t timer_id);

/**
 * @defgroup semaphore_management semaphore_management
 * Semaphore API between threads include create, wait, release, delete.
 */

/**
 * @ingroup semaphore_management
 * @brief Create and Initialize a Semaphore object used for managing resources
 *
 * @param[in] count The number of available resources
 * @return semaphore ID
 */
extern uint32_t os_semaphore_create(int32_t count);

/**
 * @ingroup semaphore_management
 * @brief Wait until a Semaphore token becomes available
 *
 * @param[in] semaphore_id semaphore id obtained from os_semaphore_create
 * @param[in] millisec timeout value
 * @return os_status code
 */
extern int32_t os_semaphore_wait(uint32_t semaphore_id, uint32_t millisec);

/**
 * @ingroup semaphore_management
 * @brief Release a Semaphore token
 *
 * @param[in] semaphore_id semaphore id obtained from os_semaphore_create
 * @return os_status code
 */
extern uint32_t os_semaphore_release(uint32_t semaphore_id);

/**
 * @ingroup semaphore_management
 * @brief Delete a Semaphore that was created by os_semaphore_create.
 *
 * @param[in] semaphore_id semaphore id obtained from os_semaphore_create
 * @return os_status code
 */
extern uint32_t os_semaphore_delete(uint32_t semaphore_id);

#ifdef __cplusplus
}
#endif

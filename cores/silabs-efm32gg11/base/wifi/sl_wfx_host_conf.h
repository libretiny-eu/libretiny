/* Copyright (c) WGM160P-LibreTiny port 2026-06-10. */
// This header is the WF200 host port's interface contract (pins + event/context
// types + entry points), not just configuration.
#pragma once

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "em_gpio.h"

#include <sl_wfx_constants.h>
#include <stdbool.h>
#include <stdint.h>

// JuiceBox-40 / WGM160P WF200 control pins (SDIO data pins live in lt_sdio.c).
#define WF200_RESET_PORT  gpioPortF
#define WF200_RESET_PIN	  12U // RESETn, active low
#define WF200_WUP_PORT	  gpioPortE
#define WF200_WUP_PIN	  4U
#define WF200_WIRQ_PORT	  gpioPortA
#define WF200_WIRQ_PIN	  8U // out-of-band IRQ (EXTI line 8 via wiring_irq)
#define WF200_LP_CLK_PORT gpioPortA
#define WF200_LP_CLK_PIN  12U // WF200 sleep clock; held low (GSDK gpio_setup)

// Indication forwarded from sl_wfx_host_post_event() to the Arduino-side
// WiFi event task. Payloads are COPIED here because the driver frees its RX
// buffer as soon as post_event returns.
typedef struct {
	uint8_t id; // SL_WFX_*_IND_ID

	union {
		struct {
			uint32_t status;
			uint8_t mac[6];
			uint16_t channel;
		} connect;

		struct {
			uint8_t mac[6];
			uint16_t reason;
		} disconnect;

		struct {
			uint8_t ssid[33];
			uint8_t ssid_len;
			uint8_t mac[6];
			uint16_t channel;
			int8_t rssi_dbm;
			uint32_t security;
		} scan_result;

		struct {
			uint32_t status;
		} scan_complete;
	} u;
} lt_wfx_event_t;

// SERIALIZATION CONTRACT: the confirmation machinery supports ONE in-flight
// command — a single waited_event_id and the shared
// wfx_context.event_payload_buffer (the buffer sl_wfx_receive_frame fills). The
// WiFi library must serialize ALL sl_wfx_* command calls behind one mutex,
// and must pass exactly the global wfx_context to sl_wfx_init().
typedef struct {
	SemaphoreHandle_t mutex;	   // sl_wfx_host_lock
	SemaphoreHandle_t wirq_sem;	   // WIRQ ISR -> bus task
	SemaphoreHandle_t confirm_sem; // setup_waited_event/wait_for_confirmation
	TaskHandle_t bus_task;
	QueueHandle_t event_queue; // lt_wfx_event_t, consumed by WiFiEvents.cpp
	volatile uint8_t waited_event_id;
	volatile bool bus_task_run;
} lt_wfx_host_t;

#ifdef __cplusplus
extern "C" {
#endif

// Defined in the host port .c file — must have C linkage.
extern lt_wfx_host_t lt_wfx_host;
extern sl_wfx_context_t wfx_context;

// Called from the Arduino GPIO ISR dispatcher (wiring_irq) on PA8 rising.
void lt_wfx_host_wirq_signal(void);
// Spawns the bus task + creates primitives; idempotent. Called before sl_wfx_init().
sl_status_t lt_wfx_host_setup(void);

#ifdef SL_WFX_USE_SECURE_LINK
// Creates the Secure Link RX mutex + renegotiation task (sl_wfx_secure_link_task.c).
// Idempotent; must run before sl_wfx_init() so the mutex exists for the
// init-time key exchange. Called from lt_wfx_host_setup().
void wfx_securelink_task_start(void);
#endif
#ifdef __cplusplus
}
#endif

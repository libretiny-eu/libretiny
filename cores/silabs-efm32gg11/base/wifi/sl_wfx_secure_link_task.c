/* Copyright (c) WGM160P-LibreTiny port 2026-06-11.
 *
 * Original FreeRTOS SecureLink renegotiation task. It owns the two kernel
 * objects the crypto hooks (sl_wfx_host_crypto.c) reference and drives the
 * public driver entry point sl_wfx_secure_link_renegotiate_session_key()
 * (Apache-2.0, secure_link/sl_wfx_secure_link.c). Two kernel objects:
 *   - wfx_securelink_rx_mutex: serializes crypto-context access between the bus
 *     RX task (decode) and the key-exchange/renegotiation path.
 *   - wfx_securelink_task_handle: the renegotiation task; the chip asks the
 *     host to renegotiate the session key after a packet-count threshold and
 *     sl_wfx_host_schedule_secure_link_renegotiation() notifies this task.
 *
 * The mutex is created in wfx_securelink_task_start(), which lt_wfx_host_setup()
 * calls BEFORE sl_wfx_init() — so the mutex exists before the first
 * sl_wfx_host_verify_pub_key()/decode during the init-time key exchange.
 */

#include "sl_wfx.h"

#ifdef SL_WFX_USE_SECURE_LINK

#include "secure_link/sl_wfx_secure_link.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#define WFX_SECURELINK_TASK_PRIO	  (configMAX_PRIORITIES - 3)
#define WFX_SECURELINK_TASK_STK_WORDS 512u

SemaphoreHandle_t wfx_securelink_rx_mutex;
TaskHandle_t wfx_securelink_task_handle;

static void wfx_securelink_task(void *arg) {
	(void)arg;
	for (;;) {
		// Block until sl_wfx_host_schedule_secure_link_renegotiation() notifies.
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		sl_status_t result = sl_wfx_secure_link_renegotiate_session_key();
		(void)result; // best-effort; a failed renegotiation surfaces as a link error
	}
}

void wfx_securelink_task_start(void) {
	if (wfx_securelink_rx_mutex == NULL) {
		wfx_securelink_rx_mutex = xSemaphoreCreateMutex();
	}
	if (wfx_securelink_task_handle == NULL) {
		xTaskCreate(
			wfx_securelink_task,
			"wfx_slk",
			WFX_SECURELINK_TASK_STK_WORDS,
			NULL,
			WFX_SECURELINK_TASK_PRIO,
			&wfx_securelink_task_handle
		);
	}
}

#endif // SL_WFX_USE_SECURE_LINK

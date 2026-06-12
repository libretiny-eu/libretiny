/* Copyright (c) WGM160P-LibreTiny port 2026-06-10.
 *
 * sl_wfx_host_* port for LibreTiny / FreeRTOS / lt_sdio on the WGM160P.
 * Plaintext SDIO build: SL_WFX_USE_SDIO defined, Secure Link compiled out.
 */

#include "sl_wfx_host_conf.h"

#include "em_gpio.h"
#include "lt_sdio.h"

#include <sl_wfx.h>
#include <sl_wfx_host_api.h>
#include <stdlib.h>
#include <string.h>

// The 1.93 MB firmware blob + PDS, flash-resident.
// CAUTION: sl_wfx_wf200_C0.h has NO include guard and DEFINES the (non-static)
// firmware array — it must be included in exactly this one translation unit.
#include <sl_wfx_wf200_C0.h>

#include "sl_wfx_pds.h"

// NOTE: no <libretiny.h> here. The silabs-wifi-port library builds in the
// family queue, BEFORE frameworks/base.py adds the public defines
// (LT_VARIANT_H etc.) that libretiny.h needs — so LT_EM/LT_IM logging is not
// available in this translation unit. WiFi-library code (Arduino queue) logs.

lt_wfx_host_t lt_wfx_host;
sl_wfx_context_t wfx_context;

// Poll gating: before sl_wfx_enable_irq() (i.e. during firmware download) no
// frame can exist AND a blind control-register poll corrupts the download
// CMD53 stream (bench 2026-06-11: download died at 33 KB with 50 ms polls).
// enable_platform_interrupt marks the bus pollable; disable clears it.
static volatile bool s_rx_pollable;

static uint32_t fw_offset;

// ---- Lifecycle --------------------------------------------------------------

sl_status_t sl_wfx_host_init(void) {
	fw_offset = 0;
	return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_deinit(void) {
	return SL_STATUS_OK;
}

// ---- Control GPIO -----------------------------------------------------------

sl_status_t sl_wfx_host_reset_chip(void) {
	// HIF_SEL: the WF200 samples SDIO_DAT2/HIF_SEL at the RISING edge of
	// RESETn — HIGH selects SDIO, LOW selects SPI (WF200 DS 4.8). On the
	// WGM160P that pin is PE9 (SDIO LOC0 — the internal WF200 bus, confirmed
	// by the module DS Table 7.1: PE8-13 are not bonded out as module pins).
	// sl_wfx_init_bus() calls reset_chip BEFORE sl_wfx_host_init_bus()
	// (sl_wfx_bus_sdio.c:99-101), so without this pre-drive the pin floats at
	// the sampling edge, the chip boots in SPI mode, and SDIO gets no response
	// ever. lt_sdio_init() reclaims the pin for the SDIO peripheral right after
	// this returns.
	GPIO_PinModeSet(gpioPortE, 9, gpioModePushPull, 1); // SDIO_DAT2/HIF_SEL (LOC0)
	GPIO_PinModeSet(WF200_RESET_PORT, WF200_RESET_PIN, gpioModePushPull, 0);
	sl_wfx_host_wait(10); // datasheet: >= 1 ms in reset
	GPIO_PinOutSet(WF200_RESET_PORT, WF200_RESET_PIN);
	sl_wfx_host_wait(25); // boot from internal ROM before first bus access
	return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_hold_in_reset(void) {
	GPIO_PinModeSet(WF200_RESET_PORT, WF200_RESET_PIN, gpioModePushPull, 0);
	return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_set_wake_up_pin(uint8_t state) {
	GPIO_PinModeSet(WF200_WUP_PORT, WF200_WUP_PIN, gpioModePushPull, state ? 1 : 0);
	return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_wait_for_wake_up(void) {
	sl_wfx_host_wait(3); // no power-save in Phase 2; token wait per ref impl
	return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_sleep_grant(
	sl_wfx_host_bus_transfer_type_t type,
	sl_wfx_register_address_t address,
	uint32_t length
) {
	(void)type;
	(void)address;
	(void)length;
	return SL_STATUS_WIFI_SLEEP_NOT_GRANTED; // WUP held awake; never sleep
}

// ---- Firmware + PDS feed ------------------------------------------------------

sl_status_t sl_wfx_host_get_firmware_size(uint32_t *firmware_size) {
	fw_offset	   = 0;
	*firmware_size = sl_wfx_firmware_size;
	return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_get_firmware_data(const uint8_t **data, uint32_t data_size) {
	// The driver clamps its last block to the remaining size; enforce it here
	// too so a flash over-read is impossible rather than trusted.
	if (fw_offset + data_size > sl_wfx_firmware_size)
		return SL_STATUS_FAIL;
	*data = &sl_wfx_firmware[fw_offset];
	fw_offset += data_size;
	return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_get_pds_size(uint16_t *pds_size) {
	// sl_wfx_pds.h (brd4321a) has no SL_WFX_PDS_KEY_COUNT macro; derive it.
	*pds_size = (uint16_t)(sizeof(sl_wfx_pds) / sizeof(sl_wfx_pds[0]));
	return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_get_pds_data(const char **pds_data, uint16_t index) {
	*pds_data = sl_wfx_pds[index];
	return SL_STATUS_OK;
}

// ---- OS primitives ------------------------------------------------------------

sl_status_t sl_wfx_host_wait(uint32_t wait_ms) {
	vTaskDelay(pdMS_TO_TICKS(wait_ms ? wait_ms : 1));
	return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_lock(void) {
	return xSemaphoreTake(lt_wfx_host.mutex, pdMS_TO_TICKS(5000)) == pdTRUE ? SL_STATUS_OK : SL_STATUS_TIMEOUT;
}

sl_status_t sl_wfx_host_unlock(void) {
	xSemaphoreGive(lt_wfx_host.mutex);
	return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_allocate_buffer(void **buffer, sl_wfx_buffer_type_t type, uint32_t buffer_size) {
	(void)type;
	// Pad to a multiple of 4: guarantees the cmd53 mod-4 over-read/write in
	// sl_wfx_host_sdio_transfer_cmd53 stays in bounds.
	*buffer = pvPortMalloc((buffer_size + 3U) & ~3UL);
	return (*buffer != NULL) ? SL_STATUS_OK : SL_STATUS_ALLOCATION_FAILED;
}

sl_status_t sl_wfx_host_free_buffer(void *buffer, sl_wfx_buffer_type_t type) {
	(void)type;
	vPortFree(buffer);
	return SL_STATUS_OK;
}

// Driver log hook (referenced when SL_WFX_DEBUG_MASK != 0; harmless otherwise).
void sl_wfx_host_log(const char *string, ...) {
	(void)string;
}

// ---- SDIO bus hooks -----------------------------------------------------------

sl_status_t sl_wfx_host_init_bus(void) {
	return (lt_sdio_init() == LT_SDIO_OK) ? SL_STATUS_OK : SL_STATUS_FAIL;
}

sl_status_t sl_wfx_host_deinit_bus(void) {
	lt_sdio_deinit();
	return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_sdio_transfer_cmd52(
	sl_wfx_host_bus_transfer_type_t type,
	uint8_t function,
	uint32_t address,
	uint8_t *buffer
) {
	lt_sdio_err_t err = lt_sdio_cmd52(type == SL_WFX_BUS_WRITE, function, address, buffer);
	return (err == LT_SDIO_OK) ? SL_STATUS_OK : SL_STATUS_FAIL;
}

sl_status_t sl_wfx_host_sdio_transfer_cmd53(
	sl_wfx_host_bus_transfer_type_t type,
	uint8_t function,
	uint32_t address,
	uint8_t *buffer,
	uint16_t buffer_length
) {
	// Byte mode (< 512): pass the driver's EXACT length through. Padding the
	// wire length is NOT harmless: the WF200 ignores queue messages whose
	// transfer length disagrees with the message header (bench 2026-06-11 —
	// padded PDS write was CRC-acked but never confirmed). lt_sdio_cmd53
	// handles the partial trailing word; buffers are 4-byte padded by
	// sl_wfx_host_allocate_buffer.
	lt_sdio_err_t err = lt_sdio_cmd53(type == SL_WFX_BUS_WRITE, function, address, buffer, buffer_length);
	return (err == LT_SDIO_OK) ? SL_STATUS_OK : SL_STATUS_FAIL;
}

sl_status_t sl_wfx_host_sdio_enable_high_speed_mode(void) {
	return (lt_sdio_enable_high_speed() == LT_SDIO_OK) ? SL_STATUS_OK : SL_STATUS_FAIL;
}

// ---- Platform interrupt (out-of-band WIRQ on PA8) ------------------------------
//
// ORDERING DEPENDENCY (Task 8 integration note): the driver calls
// sl_wfx_host_enable_platform_interrupt() from sl_wfx_enable_irq() inside
// sl_wfx_init(), i.e. possibly BEFORE the sketch ran attachInterrupt(PA8).
// Everything done here is safe in that window: pin mode, EXTI routing and the
// GPIO IEN bit are configured, but the NVIC GPIO_EVEN IRQ is deliberately NOT
// enabled here — that belongs to attachInterrupt() via the wiring_irq
// dispatcher. Without attachInterrupt() the dispatcher has no line-8 callback
// and would clear WIRQ interrupts without signaling anyone, so sketches MUST
// call attachInterrupt(PA8, lt_wfx_host_wirq_signal trampoline, RISING)
// BEFORE sl_wfx_init() or the startup indication is missed.
//
// WF200-SIDE ROUTING: the WF200 defaults to in-band DAT1 interrupts, which
// lt_sdio does not support. SL_WFX_CONFIG_CLEAR_INT_BIT (bit 15 of the CONFIG
// register) routes interrupts to the out-of-band WIRQ pin instead — the GSDK
// reference does the same in sl_wfx_host_switch_to_wirq()
// (wfx_host/sdio/sl_wfx_host_sdio_io.c). The write happens here because
// enable_platform_interrupt runs inside sl_wfx_enable_irq() after firmware
// download, with no concurrent bus traffic to race the read-modify-write.

sl_status_t sl_wfx_host_enable_platform_interrupt(void) {
	// WIRQ pin is a pulled-down input (GSDK ref: wfx_host/sl_wfx_host_init.c).
	// Nothing else in the family configures PA8 — without an input mode the
	// EXTI edge detector never sees the line.
	GPIO_PinModeSet(WF200_WIRQ_PORT, WF200_WIRQ_PIN, gpioModeInputPull, 0);
	// Route EXTI line 8 <- PA8, rising edge, regardless of whether
	// attachInterrupt() ran yet (it performs the same EXTI config).
	// enable=false here: clear any stale pending flag before setting IEN.
	GPIO_ExtIntConfig(WF200_WIRQ_PORT, WF200_WIRQ_PIN, WF200_WIRQ_PIN, true, false, false);
	GPIO_IntClear(1U << WF200_WIRQ_PIN);
	GPIO_IntEnable(1U << WF200_WIRQ_PIN);
	// Route the WF200 data IRQ to WIRQ (see block comment above). Fail hard on
	// a read error: without this bit the chip keeps signaling in-band on DAT1
	// and no interrupt ever reaches us — silently continuing would just defer
	// the failure to an opaque init timeout.
	uint32_t cfg = 0;
	if (sl_wfx_reg_read_32(SL_WFX_CONFIG_REG_ID, &cfg) != SL_STATUS_OK)
		return SL_STATUS_FAIL;
	sl_status_t ws = sl_wfx_reg_write_32(SL_WFX_CONFIG_REG_ID, cfg | SL_WFX_CONFIG_CLEAR_INT_BIT);
	if (ws == SL_STATUS_OK)
		s_rx_pollable = true;
	return ws;
}

sl_status_t sl_wfx_host_disable_platform_interrupt(void) {
	s_rx_pollable = false;
	GPIO_IntDisable(1U << WF200_WIRQ_PIN);
	return SL_STATUS_OK;
}

// Called from the wiring_irq dispatcher (registered by WiFiEvents.cpp) — ISR context.
void lt_wfx_host_wirq_signal(void) {
	BaseType_t woken = pdFALSE;
	if (lt_wfx_host.wirq_sem)
		xSemaphoreGiveFromISR(lt_wfx_host.wirq_sem, &woken);
	portYIELD_FROM_ISR(woken);
}

// ---- Confirmation / event machinery ---------------------------------------------
//
// Locking shape (verified against the vendored driver, sl_wfx.c):
// sl_wfx_send_request() takes sl_wfx_host_lock(), calls
// setup_waited_event(cmd_id), transmits, then UNLOCKS before the caller
// invokes wait_for_confirmation() — so the requester never holds the host
// lock while blocked. sl_wfx_receive_frame() (bus task) takes the host lock
// itself and calls post_event() under it, then frees the RX buffer — hence
// post_event must be non-blocking and must COPY any payload it keeps.
// No deadlock: lock-held sections never wait on each other's semaphores.

sl_status_t sl_wfx_host_setup_waited_event(uint8_t event_id) {
	// Drain a stale give left by a previous wait_for_confirmation() timeout
	// (late confirmation arrived after the requester gave up) so the next
	// wait cannot return immediately with an old payload. No give can race
	// this: post_event() runs under the host lock, and this is called either
	// under that lock (sl_wfx_send_request) or before the chip can produce
	// frames (sl_wfx_init startup-indication setup, IRQs not yet enabled).
	xSemaphoreTake(lt_wfx_host.confirm_sem, 0);
	lt_wfx_host.waited_event_id = event_id;
	return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_wait_for_confirmation(uint8_t confirmation_id, uint32_t timeout_ms, void **event_payload_out) {
	if (xSemaphoreTake(lt_wfx_host.confirm_sem, pdMS_TO_TICKS(timeout_ms)) != pdTRUE)
		return SL_STATUS_TIMEOUT;
	if (event_payload_out != NULL)
		*event_payload_out = wfx_context.event_payload_buffer;
	(void)confirmation_id; // id matching already done in post_event
	return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_transmit_frame(void *frame, uint32_t frame_len) {
	return sl_wfx_data_write(frame, frame_len);
}

// RX data frames are handed straight to the netif (Task 10 provides this; weak
// no-op until then so Task 8's WF200Init links without lwIP).
// Contract for the real implementation: `ind` is valid only for the duration
// of the call — the driver frees the RX buffer as soon as post_event returns —
// so the frame must be copied synchronously (into a pbuf). It runs with the
// host lock held (sl_wfx_receive_frame takes it), so it must not call driver
// APIs that take the host lock.
__attribute__((weak)) void lt_wf200_netif_input(sl_wfx_received_ind_t *ind) {
	(void)ind;
}

sl_status_t sl_wfx_host_post_event(sl_wfx_generic_message_t *event_payload) {
	switch (event_payload->header.id) {
		case SL_WFX_RECEIVED_IND_ID:
			lt_wf200_netif_input((sl_wfx_received_ind_t *)event_payload);
			break;

		case SL_WFX_CONNECT_IND_ID: {
			sl_wfx_connect_ind_t *ind = (sl_wfx_connect_ind_t *)event_payload;
			lt_wfx_event_t ev		  = {.id = SL_WFX_CONNECT_IND_ID};
			ev.u.connect.status		  = ind->body.status;
			ev.u.connect.channel	  = ind->body.channel;
			memcpy(ev.u.connect.mac, ind->body.mac, 6);
			if (lt_wfx_host.event_queue)
				xQueueSend(lt_wfx_host.event_queue, &ev, 0);
			break;
		}

		case SL_WFX_DISCONNECT_IND_ID: {
			sl_wfx_disconnect_ind_t *ind = (sl_wfx_disconnect_ind_t *)event_payload;
			lt_wfx_event_t ev			 = {.id = SL_WFX_DISCONNECT_IND_ID};
			ev.u.disconnect.reason		 = ind->body.reason;
			memcpy(ev.u.disconnect.mac, ind->body.mac, 6);
			if (lt_wfx_host.event_queue)
				xQueueSend(lt_wfx_host.event_queue, &ev, 0);
			break;
		}

		case SL_WFX_SCAN_RESULT_IND_ID: {
			sl_wfx_scan_result_ind_t *ind = (sl_wfx_scan_result_ind_t *)event_payload;
			lt_wfx_event_t ev			  = {.id = SL_WFX_SCAN_RESULT_IND_ID};
			// ssid_def.ssid_length is uint32_t in sl_wfx_cmd_api.h; clamp
			// before narrowing.
			uint32_t len = ind->body.ssid_def.ssid_length;
			if (len > 32)
				len = 32;
			memcpy(ev.u.scan_result.ssid, ind->body.ssid_def.ssid, len);
			ev.u.scan_result.ssid[len] = 0;
			ev.u.scan_result.ssid_len  = (uint8_t)len;
			ev.u.scan_result.channel   = ind->body.channel;
			// security_mode is a 1-byte packed bitfield struct
			// (sl_wfx_security_mode_bitmask_t): bit0 wep, bit1 wpa, bit2 wpa2,
			// bit3 wpa3, bit4 pmf, bit5 h2e, bit6 psk, bit7 eap. Store the
			// raw byte in the event's uint32_t.
			uint8_t sec = 0;
			memcpy(&sec, &ind->body.security_mode, sizeof(sec));
			ev.u.scan_result.security = sec;
			// RCPI -> dBm: dBm = rcpi/2 - 110 (WF200 API reference).
			ev.u.scan_result.rssi_dbm = (int8_t)((int32_t)ind->body.rcpi / 2 - 110);
			memcpy(ev.u.scan_result.mac, ind->body.mac, 6);
			if (lt_wfx_host.event_queue)
				xQueueSend(lt_wfx_host.event_queue, &ev, 0);
			break;
		}

		case SL_WFX_SCAN_COMPLETE_IND_ID: {
			sl_wfx_scan_complete_ind_t *ind = (sl_wfx_scan_complete_ind_t *)event_payload;
			lt_wfx_event_t ev				= {.id = SL_WFX_SCAN_COMPLETE_IND_ID};
			ev.u.scan_complete.status		= ind->body.status;
			if (lt_wfx_host.event_queue)
				xQueueSend(lt_wfx_host.event_queue, &ev, 0);
			break;
		}

		default:
			break;
	}

#ifdef SL_WFX_USE_SECURE_LINK
	// Secure Link pub-key exchange indication can land in this bus task BEFORE
	// sl_wfx_secure_link_renegotiate_session_key() arms its wait: that function
	// sends the EXCHANGE_PUB_KEYS request, THEN calls setup_waited_event() — a
	// fast bus delivers the chip's reply in the window between. When it is not
	// the currently-waited event, stash it into the driver's exchange-ind buffer
	// so renegotiate's "indication already in (can happen with slow bus)" path
	// (sl_wfx_secure_link.c) consumes it instead of blocking. Mirrors the GSDK
	// reference sl_wfx_host_post_event. WITHOUT this, the handshake loses the
	// early indication and sl_wfx_init() intermittently returns SL_STATUS_TIMEOUT
	// (0x07) — radio inits but every scan/join then fails (bench 2026-06-12).
	if (event_payload->header.id == SL_WFX_SECURELINK_EXCHANGE_PUB_KEYS_IND_ID &&
		lt_wfx_host.waited_event_id != SL_WFX_SECURELINK_EXCHANGE_PUB_KEYS_IND_ID &&
		event_payload->header.length <= sizeof(wfx_context.secure_link_exchange_ind)) {
		memcpy(&wfx_context.secure_link_exchange_ind, event_payload, event_payload->header.length);
	}
#endif

	// Confirmation the driver is blocking on? Clear the waited id BEFORE the
	// give: a duplicate confirmation can then never double-give (message ids
	// are never 0, so 0 is a safe "none" sentinel). Give only after a
	// successful copy — an oversize payload (cannot happen for the small
	// confirmation bodies, but be safe) makes the requester time out rather
	// than read stale data.
	if (lt_wfx_host.waited_event_id == event_payload->header.id) {
		lt_wfx_host.waited_event_id = 0;
		if (event_payload->header.length <= sizeof(wfx_context.event_payload_buffer)) {
			memcpy(wfx_context.event_payload_buffer, event_payload, event_payload->header.length);
			xSemaphoreGive(lt_wfx_host.confirm_sem);
		}
	}
	return SL_STATUS_OK;
}

// ---- Bus task -------------------------------------------------------------------

static void wfx_bus_task(void *arg) {
	(void)arg;
	while (lt_wfx_host.bus_task_run) {
		// WIRQ edge = fast path. Timeout = poll path: the WF200 does NOT
		// reliably signal on the OOB WIRQ pin before sleep/PDS configuration
		// (GSDK gates sl_wfx_host_switch_to_wirq on SLEEP_ENABLED and its
		// default WGM160P examples use in-band DAT1 interrupts instead, which
		// lt_sdio does not service) — bench 2026-06-11: the startup
		// indication arrives with WIRQ never rising. An empty poll is one
		// control-register read returning NO_PACKET; 50 ms keeps polled
		// confirmations responsive. Polling is gated on s_rx_pollable — see
		// its declaration for why blind polls are destructive mid-download.
		if (xSemaphoreTake(lt_wfx_host.wirq_sem, pdMS_TO_TICKS(50)) != pdTRUE && !s_rx_pollable)
			continue;
		// Drain until the piggy-backed control register reports no further
		// frame (ctrl_reg carries over so the driver can skip re-reading the
		// control register). Stop on bus error rather than spinning.
		sl_status_t result;
		uint16_t ctrl_reg = 0;
		do {
			result = sl_wfx_receive_frame(&ctrl_reg);
		} while ((ctrl_reg & SL_WFX_CONT_NEXT_LEN_MASK) != 0 && result == SL_STATUS_OK);
	}
	lt_wfx_host.bus_task = NULL;
	vTaskDelete(NULL);
}

sl_status_t lt_wfx_host_setup(void) {
	if (lt_wfx_host.mutex == NULL) {
		lt_wfx_host.mutex		= xSemaphoreCreateMutex();
		lt_wfx_host.wirq_sem	= xSemaphoreCreateBinary();
		lt_wfx_host.confirm_sem = xSemaphoreCreateBinary();
		// 32 deep: a dense home scan can burst 20+ SCAN_RESULT indications
		// faster than the WiFi event task drains them; xQueueSend(.., 0)
		// drops on overflow, so size for the worst realistic burst.
		lt_wfx_host.event_queue = xQueueCreate(32, sizeof(lt_wfx_event_t));

		// WGM160P module-internal GPIO parking — REQUIRED before the WF200 is
		// reset/enumerated. GSDK's gpio_setup() does exactly this and ONLY for
		// the WGM160P (#ifdef WGM160PX22KGA2 in wfx_host/sl_wfx_host_init.c):
		// PD0-3 and PD5 are bonded internally inside the SiP (they're absent
		// from the WGM160P datasheet pin table, like PE8-13) and tie to WF200
		// nets that must be held defined — left floating, the WF200 never
		// answers CMD5. Park them input-disabled with the weak pull-up, exactly
		// as the reference does. LP_CLK (PA12) is likewise driven low here.
		// The GPIO clock is already running by the time WiFi comes up.
		GPIO_PinModeSet(gpioPortD, 0, gpioModeDisabled, 1);
		GPIO_PinModeSet(gpioPortD, 1, gpioModeDisabled, 1);
		GPIO_PinModeSet(gpioPortD, 2, gpioModeDisabled, 1);
		GPIO_PinModeSet(gpioPortD, 3, gpioModeDisabled, 1);
		GPIO_PinModeSet(gpioPortD, 5, gpioModeDisabled, 1);
		GPIO_PinModeSet(WF200_LP_CLK_PORT, WF200_LP_CLK_PIN, gpioModePushPull, 0);
		// WUP defined low at setup (GSDK gpio_setup) — not floating during the
		// reset/CMD5 window. reset_chip drives RESETn; set_wake_up_pin drives
		// this later when the driver wants the chip awake.
		GPIO_PinModeSet(WF200_WUP_PORT, WF200_WUP_PIN, gpioModePushPull, 0);
	}
	if (!lt_wfx_host.mutex || !lt_wfx_host.wirq_sem || !lt_wfx_host.confirm_sem || !lt_wfx_host.event_queue)
		return SL_STATUS_ALLOCATION_FAILED;
#ifdef SL_WFX_USE_SECURE_LINK
	// Create the Secure Link RX mutex + renegotiation task before sl_wfx_init()
	// runs the key exchange (the crypto hooks take the mutex during it).
	wfx_securelink_task_start();
#endif
	if (lt_wfx_host.bus_task == NULL) {
		lt_wfx_host.bus_task_run = true;
		if (xTaskCreate(wfx_bus_task, "wfx_bus", 512, NULL, configMAX_PRIORITIES - 2, &lt_wfx_host.bus_task) != pdPASS)
			return SL_STATUS_ALLOCATION_FAILED;
	}
	return SL_STATUS_OK;
}

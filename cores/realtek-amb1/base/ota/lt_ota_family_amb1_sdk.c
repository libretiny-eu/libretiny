/* Strong AMB1 SDK binding for the family OTA flash seam.
 *
 * INTEGRITY ONLY: see image2_integrity.h. This file authorizes flash mutations
 * but does not authenticate the uploaded image.
 *
 * The reviewed geometry below duplicates lt_ota_family_amb1.c on purpose: the
 * bridge must stay untouched, and this file must be able to refuse to bind even
 * if the bridge geometry is later edited. Keep both in lockstep with
 * libretiny/boards/wcata007-rtl8711am-8mb.json and
 * fridge-local/tools/amb1/ota_layout.py.
 */
#include "lt_ota_family_amb1_sdk.h"

#include "lt_ota_family_amb1.h"

#if LT_AMB1_EXPERIMENTAL_IMAGE2_SDK_ADAPTER
#include "image2_sdk_adapter.h"

#include <FreeRTOS.h>
#include <ameba_soc.h>
#include <device_lock.h>
#include <flash_api.h>
#include <string.h>
#include <task.h>
#endif

#define AMB1_FLASH_SIZE	  0x00800000u
#define AMB1_SLOT_A_START 0x0000B000u
#define AMB1_SLOT_A_END	  0x0010B000u
#define AMB1_SLOT_B_START 0x00177000u
#define AMB1_SLOT_B_END	  0x00275000u
#define AMB1_KVS_START	  0x00275000u
#define AMB1_KVS_END	  0x00277000u
#define AMB1_BOOT_START	  0x00000000u
#define AMB1_BOOT_END	  0x0000B000u

/* Link extraction anchor. The bridge's weak default already satisfies the seam
 * reference, so GNU ld would not otherwise pull the strong definition out of
 * the static archive. The family builder passes
 * -Wl,--undefined=lt_ota_family_amb1_sdk_anchor; keep this symbol unique. */
const int lt_ota_family_amb1_sdk_anchor = 0;

#if LT_AMB1_EXPERIMENTAL_IMAGE2_SDK_ADAPTER

enum {
	SDK_PREPARE_UNSET = 0,
	SDK_PREPARE_READY,
	SDK_PREPARE_FAILED,
};

enum {
	SDK_BIND_UNSET = 0,
	SDK_BIND_READY,
	SDK_BIND_FAILED,
};

static const uint8_t new_signature[8] = {'8', '1', '9', '5', '8', '7', '1', '1'};
static const uint8_t old_signature[8] = {'0', '1', '9', '5', '8', '7', '1', '1'};
static const uint8_t erased[8]		  = {255, 255, 255, 255, 255, 255, 255, 255};

static int prepare_state;
static int bind_state;
static unsigned bound_running;
static image2_flash_sdkops bound_ops;
/* Private zeroed SDK object, the same pattern the adapter uses for raw reads.
 * This path must never go through the adapter's one-shot ops->read. */
static flash_t probe_flash;

int lt_ota_family_amb1_sdk_prepare(void) {
	if (prepare_state == SDK_PREPARE_UNSET)
		prepare_state = device_mutex_prepare(RT_DEV_LOCK_FLASH) == 1 ? SDK_PREPARE_READY : SDK_PREPARE_FAILED;
	return prepare_state == SDK_PREPARE_READY;
}

static int read_signature(uint32_t address, uint8_t out[8]) {
	int ok;
	device_mutex_lock(RT_DEV_LOCK_FLASH);
	ok = flash_stream_read(&probe_flash, address, 8u, out) == 1;
	device_mutex_unlock(RT_DEV_LOCK_FLASH);
	return ok;
}

/* One-shot binding. Failure latches before any early exit, so a later call can
 * never observe a partially configured adapter. */
static const image2_flash_sdkops *bind_once(void) {
	image2_sdk_range deny[3];
	image2_sdk_qualification_policy policy;
	uint8_t sig[8];
	unsigned running, target;
	uint32_t running_start, running_end;
	uint32_t target_start, target_end;

	if (bind_state != SDK_BIND_UNSET)
		return bind_state == SDK_BIND_READY ? &bound_ops : NULL;
	bind_state = SDK_BIND_FAILED;

	if (!lt_ota_family_amb1_sdk_prepare())
		return NULL;
	/* The adapter pins the current task; it must exist and this must not be an
	 * ISR. Never retry from a different task after a failure. */
	if (__get_IPSR() != 0 || !xTaskGetCurrentTaskHandle())
		return NULL;

	running = lt_ota_dual_get_current();
	if (running > 1)
		return NULL;
	target = 1u - running;

	/* Same index mapping as lt_ota_family_amb1.c: current 0 is slot A and
	 * current 1 is slot B. */
	running_start = running ? AMB1_SLOT_B_START : AMB1_SLOT_A_START;
	running_end	  = running ? AMB1_SLOT_B_END : AMB1_SLOT_A_END;
	target_start  = target ? AMB1_SLOT_B_START : AMB1_SLOT_A_START;
	target_end	  = target ? AMB1_SLOT_B_END : AMB1_SLOT_A_END;

	/* Selector evidence, mirroring ota_layout.plan(): a wrong/stale running-slot
	 * report cannot authorize that slot because its signature will not match. */
	if (!read_signature(running_start + 8u, sig) || memcmp(sig, new_signature, 8))
		return NULL;
	if (!read_signature(target_start + 8u, sig))
		return NULL;
	if (memcmp(sig, old_signature, 8) && memcmp(sig, erased, 8))
		return NULL;

	/* Deny the slot that actually holds the running image (derived above, never
	 * hard-coded), plus the boot and KVS regions. A mis-detected running slot
	 * therefore still cannot authorize its own region by accident. */
	deny[0] = (image2_sdk_range){running_start, running_end};
	deny[1] = (image2_sdk_range){AMB1_BOOT_START, AMB1_BOOT_END};
	deny[2] = (image2_sdk_range){AMB1_KVS_START, AMB1_KVS_END};

	memset(&policy, 0, sizeof(policy));
	policy.read_start = 0;
	/* Reads cover the whole flash: the running slot, the inactive readback and
	 * read-only boot/KVS inspection. Reads never authorize a mutation. */
	policy.read_size   = AMB1_FLASH_SIZE;
	policy.staging	   = (image2_sdk_range){target_start, target_end};
	policy.deny_ranges = deny;
	policy.deny_count  = 3;
	/* The running slot is a third capability used ONLY by the dedicated demote
	 * op: one ASCII '8' -> '0' at running_start + 8 after activation publishes
	 * the target. Generic program/erase hard-deny the whole source range. The
	 * demote also requires the inactive slot to have published the new
	 * signature at target_start + 8, so the adapter alone fails closed. */
	policy.source					= (image2_sdk_range){running_start, running_end};
	policy.demote_address			= running_start + 8u;
	policy.target_signature_address = target_start + 8u;
	if (!image2_sdk_adapter_bind(&policy, &bound_ops))
		return NULL;

	bound_running = running;
	bind_state	  = SDK_BIND_READY;
	return &bound_ops;
}

const image2_flash_sdkops *lt_ota_family_amb1_sdkops(void) {
	const image2_flash_sdkops *ops = bind_once();
	if (!ops)
		return NULL;
	/* Hand-off re-check: the boot source must still be the slot observed at
	 * bind. Any disagreement fails closed instead of staging into a slot that
	 * may have become the running image. */
	if (lt_ota_dual_get_current() != bound_running)
		return NULL;
	return ops;
}

#else /* !LT_AMB1_EXPERIMENTAL_IMAGE2_SDK_ADAPTER */

int lt_ota_family_amb1_sdk_prepare(void) {
	return 0;
}

const image2_flash_sdkops *lt_ota_family_amb1_sdkops(void) {
	return NULL;
}

#endif

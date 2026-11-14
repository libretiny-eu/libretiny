/* AMB1 (RTL8711AM / RTL8195A) dual-OTA boot source.
 *
 * The native image2 loader selects a slot by signature, not by address: AMB1
 * images are copied from flash into SDRAM (misc/rlx8195A-symbol-v02-img2.ld
 * places .text at 0x30000000), so the AmebaZ trick of deriving the slot from
 * the flash address of a running function cannot work here.
 *
 * Each candidate slot carries an 8-byte ASCII signature at slot + 8:
 *   "81958711" - image selected to boot ("new")
 *   "01958711" - demoted image ("old")
 * Exactly one new signature therefore identifies the running slot.
 *
 * Everything else is ambiguous for a running-slot query and fails closed to
 * slot A (index 0):
 *   - both new: torn/duplicated selector state. The loader would boot the
 *     second candidate, but which image is executing is then unproven;
 *   - neither new (both old, erased, garbage, torn): no bootable selector
 *     state that this probe is willing to interpret;
 *   - a failed read: the flash state is unknown.
 *
 * The answer is latched on first use. The running image cannot change without
 * a reboot, while an activation during this boot moves the new signature to
 * the other slot; re-deriving after that would report the just-activated image
 * as running although the old image is still executing.
 *
 * INTEGRITY ONLY: this reads a selector mark, it does not authenticate the
 * image. See image2_integrity.h.
 *
 * Index convention: 0 = slot A (ota1), 1 = slot B (ota2). This matches
 * lt_ota_family_amb1.c and image2_flash_backend.c, NOT the 1/2 convention used
 * by the AmebaZ and Beken ports.
 */
#include <libretiny.h>

#include <device_lock.h>
#include <flash_api.h>
#include <string.h>

/* WCATA007 / RTL8711AM 8 MiB geometry, matching
 * libretiny/boards/wcata007-rtl8711am-8mb.json and
 * fridge-local/tools/amb1/ota_layout.py. Only the slot starts are needed here;
 * the probe reads a fixed 8-byte signature and never indexes the slot body. */
#define AMB1_SLOT_A_START	  0x0000B000u
#define AMB1_SLOT_B_START	  0x00177000u
#define AMB1_SIGNATURE_OFFSET 8u
#define AMB1_SIGNATURE_SIZE	  8u

static const uint8_t amb1_signature_new[AMB1_SIGNATURE_SIZE] = {
	'8',
	'1',
	'9',
	'5',
	'8',
	'7',
	'1',
	'1',
};

/* Private zeroed SDK object, the same pattern the image2 SDK binding uses for
 * raw reads: flash_stream_read() only consumes its address. */
static flash_t amb1_probe_flash;

/* Cached result: 0 = slot A, 1 = slot B, -1 = not probed yet. */
static int amb1_running_slot = -1;

static uint32_t amb1_slot_start(uint8_t index) {
	return index ? AMB1_SLOT_B_START : AMB1_SLOT_A_START;
}

/* The caller must not already hold RT_DEV_LOCK_FLASH. The SDK reports success
 * as exactly 1, not as a byte count. */
static int amb1_read_signature(uint32_t address, uint8_t out[AMB1_SIGNATURE_SIZE]) {
	int ok;
	device_mutex_lock(RT_DEV_LOCK_FLASH);
	ok = flash_stream_read(&amb1_probe_flash, address, AMB1_SIGNATURE_SIZE, out) == 1;
	device_mutex_unlock(RT_DEV_LOCK_FLASH);
	return ok;
}

/* Pure selector decision. Exactly one new signature identifies the slot; every
 * other combination (including unreadable slots) resolves to slot A. */
static uint8_t amb1_choose_slot(
	const uint8_t a[AMB1_SIGNATURE_SIZE],
	const uint8_t b[AMB1_SIGNATURE_SIZE],
	int have_a,
	int have_b
) {
	if (have_a && have_b) {
		int new_a = memcmp(a, amb1_signature_new, AMB1_SIGNATURE_SIZE) == 0;
		int new_b = memcmp(b, amb1_signature_new, AMB1_SIGNATURE_SIZE) == 0;
		if (new_b && !new_a)
			return 1;
	}
	return 0;
}

lt_ota_type_t lt_ota_get_type(void) {
	return OTA_TYPE_DUAL;
}

/* Reports whether the slot currently carries the signature the loader boots.
 * This is exactly the selector mark, so it agrees with lt_ota_dual_get_current()
 * for the slot that is running and with the next boot after an activation. */
bool lt_ota_is_valid(uint8_t index) {
	uint8_t sig[AMB1_SIGNATURE_SIZE];
	if (index > 1)
		return false;
	if (!amb1_read_signature(amb1_slot_start(index) + AMB1_SIGNATURE_OFFSET, sig))
		return false;
	return memcmp(sig, amb1_signature_new, AMB1_SIGNATURE_SIZE) == 0;
}

uint8_t lt_ota_dual_get_current(void) {
	uint8_t a[AMB1_SIGNATURE_SIZE] = {0};
	uint8_t b[AMB1_SIGNATURE_SIZE] = {0};
	int have_a, have_b;

	if (amb1_running_slot >= 0)
		return (uint8_t)amb1_running_slot;

	have_a = amb1_read_signature(AMB1_SLOT_A_START + AMB1_SIGNATURE_OFFSET, a);
	have_b = amb1_read_signature(AMB1_SLOT_B_START + AMB1_SIGNATURE_OFFSET, b);

	amb1_running_slot = amb1_choose_slot(a, b, have_a, have_b);
	return (uint8_t)amb1_running_slot;
}

/* Not implemented on purpose. The loader exposes no stored-index register this
 * port can read back safely, and deriving it from the signature would claim the
 * next boot has already happened. Keep the fail-closed stub (slot A). */
uint8_t lt_ota_dual_get_stored(void) {
	return 0;
}

/* Not implemented on purpose: AMB1 activation is a family-owned signature
 * operation (lt_ota_family_end/image2_session_activate), not a flag switch. */
bool lt_ota_switch(bool revert) {
	(void)revert;
	return false;
}

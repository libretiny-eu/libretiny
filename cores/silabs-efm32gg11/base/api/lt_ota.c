/* Copyright (c) WGM160P-LibreTiny port 2026-06-12.
 *
 * Dual-bank OTA API for silabs-efm32gg11 over the ping-pong metadata layout
 * (lt_ota_meta.*). Bank A @ 0x008000, bank B @ 0x100000; metadata slots at
 * 0x1F8000 / 0x1F9000. Staged-image length comes from the FAL write-extent
 * watermark; the running image's own length comes from the linker.
 *
 * The common cores/common/base/api/lt_ota.c supplies lt_ota_begin / lt_ota_end
 * / lt_ota_write / lt_ota_write_block / lt_ota_can_rollback /
 * lt_ota_get_uf2_scheme plus the weak lt_ota_set_write_protect /
 * lt_ota_confirm. This family overrides lt_ota_get_type / lt_ota_is_valid /
 * lt_ota_dual_get_current / lt_ota_dual_get_stored / lt_ota_switch /
 * lt_ota_confirm.
 */

#include "lt_crc32.h"
#include "lt_family.h"
#include "lt_ota_meta.h"

#include <em_device.h> // SCB->VTOR
#include <fal.h>
#include <libretiny.h>
#include <string.h>

// Linker-provided length of THIS app's loadable image (see efm32gg11b820.ld).
extern char _lt_image_size[];
// FAL port: bytes staged into a bank since its last erase (0 if none).
extern uint32_t lt_fal_ota_written(uint8_t bank);

// Metadata flash access via the FAL device (the bench-proven path).
static int ota_read(uint32_t off, void *buf, uint32_t len) {
	const struct fal_flash_dev *d = fal_flash_device_find(FAL_FLASH_DEV_NAME);
	return (d && d->ops.read((long)off, buf, len) >= 0) ? 0 : -1;
}

static int ota_erase(uint32_t off, uint32_t len) {
	const struct fal_flash_dev *d = fal_flash_device_find(FAL_FLASH_DEV_NAME);
	return (d && d->ops.erase((long)off, len) >= 0) ? 0 : -1;
}

static int ota_write(uint32_t off, const void *buf, uint32_t len) {
	const struct fal_flash_dev *d = fal_flash_device_find(FAL_FLASH_DEV_NAME);
	return (d && d->ops.write((long)off, buf, len) >= 0) ? 0 : -1;
}

static const lt_ota_flash_ops_t ota_flash_ops = {ota_read, ota_erase, ota_write};

static uint8_t running_bank(void) {
	// Each app's SystemInit sets VTOR to its own bank base.
	return (SCB->VTOR >= LT_OTA_BANK_B_OFF) ? 1 : 0;
}

static uint32_t bank_crc(uint8_t bank, uint32_t len) {
	uint32_t base = lt_ota_bank_offset(bank), crc = 0xFFFFFFFFu, off = 0;
	uint8_t buf[256];
	while (len) {
		uint32_t n = len < sizeof(buf) ? len : sizeof(buf);
		if (ota_read(base + off, buf, n) != 0)
			return 0;
		crc = lt_crc32(crc, buf, n);
		off += n;
		len -= n;
	}
	return crc ^ 0xFFFFFFFFu;
}

lt_ota_type_t lt_ota_get_type(void) {
	return OTA_TYPE_DUAL;
}

uint8_t lt_ota_dual_get_current(void) {
	return running_bank() + 1; // LibreTiny dual API is 1-based
}

uint8_t lt_ota_dual_get_stored(void) {
	lt_ota_meta_t m;
	if (lt_ota_meta_load(&ota_flash_ops, &m))
		return m.boot_bank + 1;
	return running_bank() + 1;
}

bool lt_ota_is_valid(uint8_t index) {
	if (index < 1 || index > 2)
		return false;
	lt_ota_meta_t m;
	if (!lt_ota_meta_load(&ota_flash_ops, &m))
		return false;
	return lt_ota_meta_bank_valid(&ota_flash_ops, &m, index - 1);
}

bool lt_ota_switch(bool revert) {
	uint8_t cur = running_bank();
	lt_ota_meta_t m;
	if (!lt_ota_meta_load(&ota_flash_ops, &m))
		memset(&m, 0, sizeof(m));

	if (revert) {
		// Re-point boot at the currently-running (good) bank; commit it.
		m.boot_bank = cur;
		m.attempts	= 0;
		if (m.bank[cur].state == LT_OTA_BANK_TRIAL)
			m.bank[cur].state = LT_OTA_BANK_CONFIRMED;
		return lt_ota_meta_store(&ota_flash_ops, &m);
	}

	uint8_t tgt	 = cur ^ 1u;
	uint32_t len = lt_fal_ota_written(tgt);
	if (len == 0 || len > LT_OTA_BANK_LEN)
		return false; // nothing staged into the inactive bank
	m.bank[tgt].length = len;
	m.bank[tgt].crc32  = bank_crc(tgt, len);
	m.bank[tgt].state  = LT_OTA_BANK_TRIAL;
	m.boot_bank		   = tgt;
	m.attempts		   = LT_OTA_TRIAL_ATTEMPTS;
	if (!lt_ota_meta_store(&ota_flash_ops, &m))
		return false;
	return lt_ota_meta_bank_valid(&ota_flash_ops, &m, tgt);
}

void lt_ota_confirm(void) {
	uint8_t cur	 = running_bank();
	uint32_t len = (uint32_t)_lt_image_size;
	if (len == 0 || len > LT_OTA_BANK_LEN)
		return;
	lt_ota_meta_t m;
	if (!lt_ota_meta_load(&ota_flash_ops, &m))
		memset(&m, 0, sizeof(m));
	m.bank[cur].length = len;
	m.bank[cur].crc32  = bank_crc(cur, len);
	m.bank[cur].state  = LT_OTA_BANK_CONFIRMED;
	m.boot_bank		   = cur;
	m.attempts		   = 0;
	lt_ota_meta_store(&ota_flash_ops, &m);
}

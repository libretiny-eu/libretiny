/* AMB1 family lifecycle hook: stage a UF2 stream into the reviewed inactive
 * image2 slot, verify its readback and activate it without rebooting.
 *
 * INTEGRITY ONLY. The crypto provider compares the received stream against the
 * staged readback; it is not authentication (see image2_integrity.h).
 *
 * Size contract (fail-closed): lt_ota_begin() receives the TOTAL UF2 file size,
 * which is block_count * 512 (one 512-byte header block plus data blocks).
 * uf2tool places only 256 image bytes in each 512-byte data block, so
 *     image_size = (size / 512 - 1) * 256
 * Every delivered data block must be exactly 256 bytes, matching that fixed
 * chunking, so a short final block, a non-512-byte total or a size that does
 * not fit the inactive slot is refused. begin() may already have erased the
 * inactive extent before a later block is refused; that is the documented,
 * recoverable cost of rejecting an invalid stream.
 *
 * Each object below is one-shot for the whole boot. A second begin()/write()/
 * end() after a terminal call is rejected; abort() is idempotent and never
 * publishes, activates or reboots.
 */
#include "lt_ota_family_amb1.h"

#include "image2_integrity.h"
#include "image2_session.h"

#include <stdint.h>
#include <string.h>

/* WCATA007 / RTL8711AM 8 MiB geometry, matching
 * libretiny/boards/wcata007-rtl8711am-8mb.json and
 * fridge-local/tools/amb1/ota_layout.py. */
#define AMB1_FLASH_SIZE	  0x00800000u
#define AMB1_SECTOR_SIZE  0x00001000u
#define AMB1_PAGE_SIZE	  0x00000100u
#define AMB1_SLOT_A_START 0x0000B000u
#define AMB1_SLOT_A_END	  0x0010B000u
#define AMB1_SLOT_B_START 0x00177000u
#define AMB1_SLOT_B_END	  0x00275000u
#define AMB1_KVS_START	  0x00275000u
#define AMB1_KVS_END	  0x00277000u
#define AMB1_BOOT_START	  0x00000000u
#define AMB1_BOOT_END	  0x0000B000u

/* Native image2 load regions, matching fridge-local/tools/amb1/verify_image2.py
 * and package_elf.py. */
#define AMB1_SRAM_BASE	0x10006000u
#define AMB1_SRAM_END	0x10070000u
#define AMB1_SDRAM_BASE 0x30000000u
#define AMB1_SDRAM_END	0x30200000u

#define AMB1_UF2_BLOCK_SIZE 512u
#define AMB1_DATA_BLOCK		256u
#define AMB1_MIN_IMAGE		42u

enum {
	AMB1_STATE_UNUSED = 0,
	AMB1_STATE_STAGING,
	AMB1_STATE_FINISHED,
	AMB1_STATE_ABORTED,
};

/* Exactly one of each per boot. Never reset, copy or reinitialize a used
 * object; the family state machine below makes the boot latch explicit. */
static image2_session_boot amb1_boot;
static image2_session amb1_session;
static image2_integrity amb1_integrity;
static image2_flash_config amb1_config;
static image2_flash_sdkops amb1_sdkops;

static unsigned amb1_running_slot;
static uint32_t amb1_image_size;
static uint32_t amb1_total_blocks;
static uint32_t amb1_staged;
static uint32_t amb1_next_seq;
static int amb1_state;
static int amb1_aborted;

__attribute__((weak)) const image2_flash_sdkops *lt_ota_family_amb1_sdkops(void) {
	return NULL;
}

/* Link extraction anchor. The weak defaults in lt_ota.c already satisfy every
 * reference to the family hooks, so GNU ld would not extract this strong
 * implementation from the static archive and the qualification build would
 * silently keep the no-op path. The builder passes
 * -Wl,--undefined=lt_ota_family_amb1_anchor for the qualification build only;
 * keep this symbol unique. */
const int lt_ota_family_amb1_anchor = 0;

static int round_extent(uint32_t size, uint32_t *end) {
	if (size > UINT32_MAX - (AMB1_SECTOR_SIZE - 1u))
		return 0;
	*end = ((size + AMB1_SECTOR_SIZE - 1u) / AMB1_SECTOR_SIZE) * AMB1_SECTOR_SIZE;
	return 1;
}

/* Build the reviewed normal-selector geometry. The target is always the slot
 * that is not running, and its capacity is the only slot allowlist admitted. */
static int build_config(uint32_t image_size, unsigned running) {
	image2_range target;
	uint32_t rounded;
	if (running > 1)
		return 0;
	target.start = running ? AMB1_SLOT_A_START : AMB1_SLOT_B_START;
	target.end	 = running ? AMB1_SLOT_A_END : AMB1_SLOT_B_END;
	if (!round_extent(image_size, &rounded))
		return 0;
	if (rounded > target.end - target.start)
		return 0;

	memset(&amb1_config, 0, sizeof(amb1_config));
	amb1_config.layout.flash_size		   = AMB1_FLASH_SIZE;
	amb1_config.layout.sector_size		   = AMB1_SECTOR_SIZE;
	amb1_config.layout.slots[0]			   = (image2_range){AMB1_SLOT_A_START, AMB1_SLOT_A_END};
	amb1_config.layout.slots[1]			   = (image2_range){AMB1_SLOT_B_START, AMB1_SLOT_B_END};
	amb1_config.layout.running_slot		   = running;
	amb1_config.layout.reviewed			   = target;
	amb1_config.layout.protected_ranges[0] = (image2_range){AMB1_KVS_START, AMB1_KVS_END};
	amb1_config.layout.protected_ranges[1] = (image2_range){AMB1_BOOT_START, AMB1_BOOT_END};
	amb1_config.layout.protected_count	   = 2;
	amb1_config.layout.sram_base		   = AMB1_SRAM_BASE;
	amb1_config.layout.sram_end			   = AMB1_SRAM_END;
	amb1_config.layout.sdram_base		   = AMB1_SDRAM_BASE;
	amb1_config.layout.sdram_end		   = AMB1_SDRAM_END;
	amb1_config.layout.reviewed_layout	   = 1;
	amb1_config.layout.normal_selector	   = 1;

	amb1_config.extent.start		= target.start;
	amb1_config.extent.end			= target.start + rounded;
	amb1_config.image_size			= image_size;
	amb1_config.page_size			= AMB1_PAGE_SIZE;
	amb1_config.allow_source_demote = 1;
	return 1;
}

/* The family's own target-slot allowlist. This runs before the stock sequence,
 * OTA_PART_INFO and BINPATCH steps, so it never trusts UF2 partition tags. */
static int in_inactive_slot(uint32_t address, uint32_t length) {
	image2_range target;
	if (amb1_running_slot > 1)
		return 0;
	target = amb1_config.layout.slots[1u - amb1_running_slot];
	if (!length || address < target.start || address > target.end)
		return 0;
	return length <= target.end - address;
}

lt_ota_family_result_t lt_ota_family_begin(lt_ota_ctx_t *ctx, size_t size) {
	const image2_flash_sdkops *ops;
	image2_session_crypto crypto;
	unsigned running;
	uint32_t image_size;

	if (amb1_state != AMB1_STATE_UNUSED)
		return UF2_ERR_WRITE_FAILED;
	amb1_state = AMB1_STATE_ABORTED; /* consume the boot latch before any exit */
	if (!ctx)
		return UF2_ERR_WRITE_FAILED;

	ops = lt_ota_family_amb1_sdkops();
	if (!ops)
		return UF2_ERR_WRITE_FAILED;

	/* Total UF2 size: a 512-byte header plus a whole number of 512-byte
	 * blocks. uf2tool emits 256-byte data payloads per block, so the exact
	 * image length is (blocks - 1) * 256, NOT size - 512. */
	if (size > (size_t)UINT32_MAX || size < (size_t)AMB1_UF2_BLOCK_SIZE + AMB1_MIN_IMAGE || size % AMB1_UF2_BLOCK_SIZE)
		return UF2_ERR_WRITE_FAILED;
	image_size = (uint32_t)((size / AMB1_UF2_BLOCK_SIZE) - 1u) * AMB1_DATA_BLOCK;

	running = lt_ota_dual_get_current();
	if (running > 1)
		return UF2_ERR_WRITE_FAILED;
	if (!build_config(image_size, running))
		return UF2_ERR_WRITE_FAILED;

	amb1_running_slot = running; /* frozen for the rest of this boot */
	amb1_image_size	  = image_size;
	amb1_total_blocks = (uint32_t)(size / AMB1_UF2_BLOCK_SIZE);
	amb1_staged		  = 0;
	amb1_next_seq	  = 1;

	amb1_sdkops = *ops;
	if (!image2_integrity_init(&amb1_integrity, image_size) || !image2_integrity_callbacks(&amb1_integrity, &crypto)) {
		image2_integrity_cleanup(&amb1_integrity);
		return UF2_ERR_WRITE_FAILED;
	}
	if (!image2_session_begin(&amb1_session, &amb1_boot, &amb1_config, &amb1_sdkops, &crypto, running)) {
		image2_integrity_cleanup(&amb1_integrity);
		return UF2_ERR_WRITE_FAILED;
	}

	amb1_state = AMB1_STATE_STAGING;
	return LT_OTA_FAMILY_HANDLED;
}

lt_ota_family_result_t lt_ota_family_write_block(lt_ota_ctx_t *ctx, const uf2_block_t *block) {
	uint32_t address;

	(void)ctx;
	if (amb1_state != AMB1_STATE_STAGING)
		return UF2_ERR_WRITE_FAILED;
	if (!block)
		goto fail;

	/* The common path only delivers main-flash blocks with data, but the
	 * family owns the decision once begin() returned HANDLED. */
	if (block->not_main_flash || block->file_container || !block->len)
		goto fail;
	/* uf2tool chunks payloads at exactly 256 bytes. A short final block is
	 * refused rather than guessed at; see the size contract above. */
	if (block->len != AMB1_DATA_BLOCK) {
		amb1_state = AMB1_STATE_ABORTED;
		return UF2_ERR_WRITE_LENGTH;
	}
	if (block->block_count != amb1_total_blocks || block->block_seq != amb1_next_seq || block->addr != amb1_staged) {
		amb1_state = AMB1_STATE_ABORTED;
		return UF2_ERR_SEQ_MISMATCH;
	}
	if (amb1_staged > amb1_image_size || block->len > amb1_image_size - amb1_staged)
		goto fail;

	/* Map the image-relative UF2 address into the reviewed inactive slot and
	 * enforce the allowlist independently of any UF2 partition metadata. */
	address = amb1_config.extent.start + block->addr;
	if (!in_inactive_slot(address, block->len) ||
		amb1_config.extent.start != amb1_config.layout.slots[1u - amb1_running_slot].start)
		goto fail;

	if (!image2_session_write(&amb1_session, block->data, block->len))
		goto fail;

	amb1_staged += block->len;
	amb1_next_seq++;
	return LT_OTA_FAMILY_HANDLED;

fail:
	amb1_state = AMB1_STATE_ABORTED;
	return UF2_ERR_WRITE_FAILED;
}

lt_ota_family_result_t lt_ota_family_end(lt_ota_ctx_t *ctx) {
	(void)ctx;
	if (amb1_state != AMB1_STATE_STAGING)
		return UF2_ERR_WRITE_FAILED;
	if (amb1_staged != amb1_image_size) {
		amb1_state = AMB1_STATE_ABORTED;
		return UF2_ERR_WRITE_FAILED;
	}
	/* Verify readback consistency, then publish + demote. No reboot and no
	 * lt_ota_switch(): activation is a signature operation only. */
	if (!image2_session_verify(&amb1_session)) {
		amb1_state = AMB1_STATE_ABORTED;
		return UF2_ERR_WRITE_FAILED;
	}
	if (!image2_session_activate(&amb1_session)) {
		amb1_state = AMB1_STATE_ABORTED;
		return UF2_ERR_WRITE_FAILED;
	}

	amb1_state = AMB1_STATE_FINISHED;
	image2_integrity_cleanup(&amb1_integrity);
	return LT_OTA_FAMILY_HANDLED;
}

void lt_ota_family_abort(lt_ota_ctx_t *ctx) {
	(void)ctx;
	/* Idempotent, and never runs after a successful end(). The state may
	 * already be ABORTED after a rejected block or failed end; cleanup still
	 * runs exactly once so a live session lock or hash context is released. */
	if (amb1_state == AMB1_STATE_UNUSED || amb1_state == AMB1_STATE_FINISHED || amb1_aborted)
		return;
	amb1_aborted = 1;
	amb1_state	 = AMB1_STATE_ABORTED;
	/* The session abort path only discards STAGING/VERIFIED state; activation
	 * is a separate call that this function never makes. */
	image2_session_abort(&amb1_session);
	image2_integrity_cleanup(&amb1_integrity);
}

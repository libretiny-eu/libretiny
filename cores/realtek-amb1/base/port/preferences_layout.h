/* WCATA007 proposed reservation policy; no flash I/O or write authorization. */
#pragma once

#include <stdint.h>

#define AMB1_PREF_FLASH_SIZE  UINT32_C(0x800000)
#define AMB1_PREF_SECTOR_SIZE UINT32_C(0x1000)
#define AMB1_PREF_SLOT_A	  UINT32_C(0x00b000)
#define AMB1_PREF_SLOT_A_END  UINT32_C(0x10b000)
#define AMB1_PREF_SLOT_B	  UINT32_C(0x177000)
#define AMB1_PREF_SLOT_B_END  UINT32_C(0x275000)
#define AMB1_PREF_KVS_START	  UINT32_C(0x275000)
#define AMB1_PREF_KVS_END	  UINT32_C(0x277000)
#define AMB1_PREF_IMAGE_MAX	  UINT32_C(0x0fe000)

/* Callers must independently establish each fact against a matching snapshot.
 * These are policy assertions, not runtime proof or a capability to mutate. */
typedef struct {
	int reviewed_reservation;
	int snapshot_matches_review;
	int migrated_native_loader;
	int normal_selector;
	int inactive_b_reservation;
	int admitted_images_exclude_kvs;
} amb1_preferences_review_t;

static inline int amb1_preferences_layout_reviewed(const amb1_preferences_review_t *r) {
	return r && r->reviewed_reservation == 1 && r->snapshot_matches_review == 1 && r->migrated_native_loader == 1 &&
		   r->normal_selector == 1 && r->inactive_b_reservation == 1 && r->admitted_images_exclude_kvs == 1;
}

/* Reject empty, overflowing and outward-rounded requests. Erase callers must
 * supply whole sectors; program callers must preserve the exact byte extent. */
static inline int amb1_preferences_range_allowed(
	const amb1_preferences_review_t *r,
	uint32_t offset,
	uint32_t length,
	int erase
) {
	if (!amb1_preferences_layout_reviewed(r) || !length || offset < AMB1_PREF_KVS_START ||
		offset >= AMB1_PREF_KVS_END || length > AMB1_PREF_KVS_END - offset)
		return 0;
	return !erase || (offset % AMB1_PREF_SECTOR_SIZE == 0 && length % AMB1_PREF_SECTOR_SIZE == 0);
}

/* Native image2 length includes both headers and all native payload bytes.
 * Outer envelopes/UF2 padding are not part of this admitted image extent. */
static inline int amb1_preferences_image_allowed(unsigned slot, uint32_t length) {
	uint32_t start, end, rounded;
	if (slot > 1 || length < 42 || length > AMB1_PREF_IMAGE_MAX)
		return 0;
	start	= slot ? AMB1_PREF_SLOT_B : AMB1_PREF_SLOT_A;
	end		= slot ? AMB1_PREF_SLOT_B_END : AMB1_PREF_SLOT_A_END;
	rounded = (length + AMB1_PREF_SECTOR_SIZE - 1) / AMB1_PREF_SECTOR_SIZE * AMB1_PREF_SECTOR_SIZE;
	return rounded <= end - start;
}

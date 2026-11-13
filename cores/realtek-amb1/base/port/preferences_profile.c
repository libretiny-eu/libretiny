/* Caller attestations only: never qualifies hardware or acquires a snapshot. */
#include "preferences_sdk.h"

#if LT_AMB1_EXPERIMENTAL_KVS && LT_AMB1_WCATA007_KVS_PROFILE
#include <libretiny.h>
#include <ameba_soc.h>
#include <device_lock.h>
#include <FreeRTOS.h>
#include <task.h>
#include <string.h>

/* Private, externally reviewed build input; never generated from a host test. */
#ifndef LT_AMB1_KVS_ATTESTATION_HEADER
#error "WCATA007 KVS requires an external attestation header"
#else
#include LT_AMB1_KVS_ATTESTATION_HEADER
#endif

#if LT_AMB1_KVS_HARDWARE_QUALIFIED != 1
#error "Qualify exact SDK extents, shared mutex and hardware before binding"
#endif
#if LT_AMB1_KVS_REVIEWED_RESERVATION != 1 || LT_AMB1_KVS_MIGRATED_NATIVE_LOADER != 1 || \
    LT_AMB1_KVS_NORMAL_SELECTOR != 1 || LT_AMB1_KVS_INACTIVE_B_RESERVATION != 1 || \
    LT_AMB1_KVS_ADMITTED_IMAGES_EXCLUDE_KVS != 1
#error "Every migration and reservation review fact must be explicit"
#endif
#if LT_AMB1_KVS_BASELINE_MIGRATION_ATTESTED != 1 || LT_AMB1_KVS_SHARED_LOCK_ATTESTED != 1 || \
    LT_AMB1_KVS_PROFILE_TABLE_REVIEWED != 1
#error "Require baseline migration, shared-lock and generated-table review"
#endif
#ifndef LT_AMB1_KVS_REVIEWED_BASELINE_SHA256_BYTES
#error "Supply the exact 32-byte externally reviewed baseline identifier"
#endif
#if !defined(LT_AMB1_KVS_EXISTING_FLASHDB_ATTESTED) || !defined(LT_AMB1_KVS_INITIAL_FORMAT_APPROVED)
#error "Explicitly select existing ownership and initial-format policy"
#endif
#if (LT_AMB1_KVS_EXISTING_FLASHDB_ATTESTED != 0 && LT_AMB1_KVS_EXISTING_FLASHDB_ATTESTED != 1) || \
    (LT_AMB1_KVS_INITIAL_FORMAT_APPROVED != 0 && LT_AMB1_KVS_INITIAL_FORMAT_APPROVED != 1) || \
    (!LT_AMB1_KVS_EXISTING_FLASHDB_ATTESTED && !LT_AMB1_KVS_INITIAL_FORMAT_APPROVED)
#error "Require existing ownership or separately approved destructive initialization"
#endif

/* These are profile constants, NOT a
 * lookup of the current FAL table. External table review is mandatory. */
_Static_assert(FLASH_LENGTH == AMB1_PREF_FLASH_SIZE, "WCATA007 requires 8 MiB");
_Static_assert(AMB1_PREF_SLOT_A_END <= AMB1_PREF_SLOT_B &&
	AMB1_PREF_SLOT_B_END == AMB1_PREF_KVS_START &&
	AMB1_PREF_KVS_END <= 0x2e3000 && 0x2e3000 < FLASH_LENGTH, "partition bounds");
_Static_assert(AMB1_PREF_KVS_START % AMB1_PREF_SECTOR_SIZE == 0 &&
	AMB1_PREF_KVS_END - AMB1_PREF_KVS_START == 2 * AMB1_PREF_SECTOR_SIZE &&
	AMB1_PREF_IMAGE_MAX <= AMB1_PREF_SLOT_B_END - AMB1_PREF_SLOT_B &&
	AMB1_PREF_IMAGE_MAX <= AMB1_PREF_SLOT_A_END - AMB1_PREF_SLOT_A, "KVS/image bounds");
static const uint8_t reviewed_baseline[] = { LT_AMB1_KVS_REVIEWED_BASELINE_SHA256_BYTES };
_Static_assert(sizeof(reviewed_baseline) == AMB1_PREF_SNAPSHOT_HASH_SIZE, "SHA-256 size");

/* Force extraction of this object from a core archive with a linker EXTERN or
 * --undefined=lt_amb1_preferences_profile_anchor; the FAL fallback is weak. */
void lt_amb1_preferences_profile_anchor(void) {}

void lt_amb1_preferences_configure(void) {
	/* A non-null current TCB alone does not prove the scheduler has started. */
	if (__get_IPSR() != 0 || xTaskGetSchedulerState() != taskSCHEDULER_RUNNING ||
	    xTaskGetCurrentTaskHandle() == NULL)
		return;
	amb1_preferences_sdk_policy_t policy = {0};
	policy.flash.review = (amb1_preferences_review_t) {
		LT_AMB1_KVS_REVIEWED_RESERVATION, LT_AMB1_KVS_BASELINE_MIGRATION_ATTESTED,
		LT_AMB1_KVS_MIGRATED_NATIVE_LOADER, LT_AMB1_KVS_NORMAL_SELECTOR,
		LT_AMB1_KVS_INACTIVE_B_RESERVATION, LT_AMB1_KVS_ADMITTED_IMAGES_EXCLUDE_KVS
	};
	/* Both identifiers name the reviewed migration BASELINE, not the newly
	 * compiled application's whole-flash hash. External review must establish
	 * freshness and allowed deltas to that baseline before enabling this flag.
	 * The adapter compares identifiers; it does not verify that evidence. */
	memcpy(policy.flash.expected_snapshot_hash, reviewed_baseline, sizeof(reviewed_baseline));
	memcpy(policy.flash.fresh_snapshot_hash, reviewed_baseline, sizeof(reviewed_baseline));
	policy.flash.fresh_snapshot_attested = LT_AMB1_KVS_BASELINE_MIGRATION_ATTESTED;
	policy.flash.shared_controller_lock_attested = LT_AMB1_KVS_SHARED_LOCK_ATTESTED;
	policy.flash.existing_flashdb_attested = LT_AMB1_KVS_EXISTING_FLASHDB_ATTESTED;
	policy.flash.initial_format_approved = LT_AMB1_KVS_INITIAL_FORMAT_APPROVED;
	policy.current_geometry = (amb1_preferences_sdk_geometry_t) {
		FLASH_LENGTH, AMB1_PREF_SECTOR_SIZE, 256,
		AMB1_PREF_SLOT_A, AMB1_PREF_SLOT_A_END, AMB1_PREF_SLOT_B, AMB1_PREF_SLOT_B_END,
		AMB1_PREF_KVS_START, AMB1_PREF_KVS_END, 0x2e3000, FLASH_LENGTH, AMB1_PREF_IMAGE_MAX
	};
	policy.runtime_qualification_passed = LT_AMB1_KVS_HARDWARE_QUALIFIED;
	if (device_mutex_prepare(RT_DEV_LOCK_FLASH) != 1) {
		LT_E("AMB1 KVS: shared flash mutex preparation failed; unbound");
		return;
	}
	if (amb1_preferences_sdk_bind(&policy) != 0)
		LT_E("AMB1 KVS: policy binding failed; no persistence approval");
}
#endif

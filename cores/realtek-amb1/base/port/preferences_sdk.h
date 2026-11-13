/* Reviewed WCATA007 KVS binding, not a flash qualification procedure. */
#ifndef AMB1_PREFERENCES_SDK_H
#define AMB1_PREFERENCES_SDK_H

#include "preferences_flash.h"

#ifndef LT_AMB1_EXPERIMENTAL_KVS
#define LT_AMB1_EXPERIMENTAL_KVS 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t flash_size, sector_size, page_size;
	uint32_t ota1_start, ota1_end, ota2_start, ota2_end;
	uint32_t kvs_start, kvs_end, system_start, system_end, image_max;
} amb1_preferences_sdk_geometry_t;

typedef struct {
	amb1_preferences_flash_policy_t flash;
	amb1_preferences_sdk_geometry_t current_geometry;
	/* External runtime qualification must cover exact SDK extents, mutex,
	 * page programming and erase/readback on this hardware/SDK/profile.
	 * A host test or matching geometry alone cannot establish this fact. */
	int runtime_qualification_passed;
} amb1_preferences_sdk_policy_t;

/* Return 0 on success, -1 on failure. Bind once with consumers quiesced,
 * AFTER BSS/heap/scheduler readiness and external shared SDK
 * device_mutex_prepare(RT_DEV_LOCK_FLASH), BEFORE FlashDB initialization.
 * Uses the patched SDK try-lock, never creates a private/lazy mutex.
 * The caller must supply every review/snapshot/ownership/format attestation;
 * this module neither acquires snapshots nor invents SHA-256 evidence.
 * Keep global SDK --wrap guards, including burst-write and sector-erase.
 * Only private, held-lock, KVS-bounded callbacks may use __real symbols.
 * No bind, FAL init, boot, or test constitutes physical mutation approval. */
int amb1_preferences_sdk_bind(const amb1_preferences_sdk_policy_t *);

/* FAL flash0 hooks: PHYSICAL offsets, success = requested length, else -1.
 * Dispatch only wholly-contained nonempty KVS requests here. Never call a
 * locking FAL read from this bridge (the shared mutex is already held).
 * Other flash reads remain with the existing read-only FAL implementation;
 * all non-KVS mutations must remain denied. Each call takes its own lock.
 * Database-level concurrency still requires a separate FlashDB mutex. */
int amb1_preferences_sdk_read(long, uint8_t *, size_t);
int amb1_preferences_sdk_write(long, const uint8_t *, size_t);
int amb1_preferences_sdk_erase(long, size_t);

/* Mandatory before fdb_kvdb_init: set FDB_KVDB_CTRL_SET_NOT_FORMAT to
 * !amb1_preferences_sdk_format_allowed(). Existing ownership alone does not
 * approve autoformat. Do not start FlashDB before successful qualification
 * and binding; default-off/unbound returns false and denies all hooks. */
int amb1_preferences_sdk_format_allowed(void);

#ifdef __cplusplus
}
#endif
#endif

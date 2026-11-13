/* Bounded preferences transport. No SDK binding or automatic initialization. */
#ifndef AMB1_PREFERENCES_FLASH_H
#define AMB1_PREFERENCES_FLASH_H

#include <stddef.h>
#include "preferences_layout.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AMB1_PREF_SNAPSHOT_HASH_SIZE 32u

typedef struct {
	void *context;
	/* Return 0 on success, nonzero on failure. Addresses are absolute offsets.
	 * Callbacks must perform synchronous, exact-extent I/O without rounding.
	 * lock/unlock MUST use the prepared SDK shared flash-controller mutex,
	 * including coordination with OTA, chip-ID, FAL and other flash users.
	 * No callback may recursively acquire that mutex. */
	int (*read)(void *, uint32_t, void *, size_t);
	int (*write)(void *, uint32_t, const void *, size_t);
	int (*erase)(void *, uint32_t, size_t);
	int (*lock)(void *);
	void (*unlock)(void *);
} amb1_preferences_flash_ops_t;

typedef struct {
	amb1_preferences_review_t review;
	/* External verifier supplies SHA-256 identifiers and attests freshness;
	 * equality here is NOT snapshot acquisition or cryptographic validation. */
	uint8_t expected_snapshot_hash[AMB1_PREF_SNAPSHOT_HASH_SIZE];
	uint8_t fresh_snapshot_hash[AMB1_PREF_SNAPSHOT_HASH_SIZE];
	int fresh_snapshot_attested;
	int shared_controller_lock_attested;
	/* Unknown data is read-only unless destructive initialization is explicitly
	 * approved. Existing FlashDB ownership must be independently established. */
	int existing_flashdb_attested;
	int initial_format_approved;
} amb1_preferences_flash_policy_t;

/* Zero-initialize before bind. Configure once while all consumers are quiesced;
 * do not rebind or mutate the structure while an operation is in progress. */
typedef struct {
	amb1_preferences_flash_ops_t ops;
	amb1_preferences_flash_policy_t policy;
	int bound;
} amb1_preferences_flash_t;

/* Failed binding clears the instance. No flash access occurs during binding. */
int amb1_preferences_flash_bind(amb1_preferences_flash_t *,
	const amb1_preferences_flash_ops_t *, const amb1_preferences_flash_policy_t *);

/* FAL device callbacks can forward here. Offsets are RELATIVE to KVS_START,
 * lengths must be nonzero, success returns length, failure returns -1.
 * A shared SDK lock is held only for this operation, including verification.
 * Higher layers still need a separate database mutex for concurrent KV calls.
 * Writes preflight NOR transitions and verify exact readback; failures can
 * leave partial programming/erase, to be recovered by FlashDB, not retried
 * blindly. These functions never format a database or touch adjacent flash. */
int amb1_preferences_flash_read(amb1_preferences_flash_t *, long, void *, size_t);
int amb1_preferences_flash_write(amb1_preferences_flash_t *, long, const void *, size_t);
int amb1_preferences_flash_erase(amb1_preferences_flash_t *, long, size_t);

/* Before fdb_kvdb_init(), set FDB_KVDB_CTRL_SET_NOT_FORMAT to the inverse
 * of this value. Mandatory even for an attested existing database: FlashDB
 * must not autoformat corrupt/foreign sectors without separate approval. */
int amb1_preferences_flash_format_allowed(const amb1_preferences_flash_t *);

#ifdef __cplusplus
}
#endif
#endif

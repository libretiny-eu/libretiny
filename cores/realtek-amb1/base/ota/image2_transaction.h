/* Experimental host-tested staging core; NOT a production OTA backend.
 * No SDK, transport, allocation, loader writes or automatic rollback.
 */
#ifndef AMB1_IMAGE2_TRANSACTION_H
#define AMB1_IMAGE2_TRANSACTION_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IMAGE2_MAX_PROTECTED 8
typedef struct { uint32_t start, end; } image2_range;
typedef enum {
	IMAGE2_IDLE, IMAGE2_STAGING, IMAGE2_VERIFIED, IMAGE2_ABORTED,
	IMAGE2_FAILED, IMAGE2_COMMIT_ATTEMPTED, IMAGE2_COMMITTED
} image2_state;

typedef struct {
	uint32_t flash_size, sector_size;
	image2_range slots[2], reviewed;
	image2_range protected_ranges[IMAGE2_MAX_PROTECTED];
	size_t protected_count;
	uint32_t sram_base, sram_end, sdram_base, sdram_end;
	unsigned running_slot;
	/* Caller attests fresh layout/loader review and reliable boot handoff.
	 * Normal selector semantics only; signatures are never a boot-source oracle.
	 */
	int reviewed_layout, normal_selector;
} image2_layout;

typedef struct {
	void *ctx;
	/* Every callback returns EXACTLY 1 on success, anything else on failure.
	 * Caller serializes ALL flash users for the entire object lifetime; callbacks
	 * must not reenter this API. Reads/writes are exact, no short successes.
	 */
	int (*read)(void *, uint32_t, void *, size_t);
	int (*erase)(void *, uint32_t, size_t);
	int (*write)(void *, uint32_t, const void *, size_t);
	/* Must authenticate metadata/length/expected digest BEFORE mutation.
	 * Hash reset/update/check implement the caller's cryptographic policy.
	 * check compares against that authenticated digest, not a learned digest.
	 * Called for received bytes and separately for reconstructed readback.
	 * Keep authenticated policy/context immutable across this transaction.
	 * Callbacks own any hash resources and must clean up idempotently; this core
	 * never allocates, acquires locks or assumes ownership of callback resources.
	 */
	int (*authorize)(void *, uint32_t);
	int (*hash_reset)(void *);
	int (*hash_update)(void *, const void *, size_t);
	int (*hash_check)(void *);
} image2_callbacks;

/* Initialize ONCE with {0}, retain for the entire boot session. Never copy,
 * zero, reinitialize or replace after use (including errors). No reset API:
 * terminal processing failures, aborts and all commit attempts permanently deny
 * mutations. Rejected begin/write/verify calls do not necessarily change state.
 * Backend capability/serialization must enforce this lifetime across clients.
 */
typedef struct {
	image2_state state;
	image2_layout layout;
	image2_callbacks io;
	uint32_t size, received, target, erase_end;
	uint8_t signature[8];
} image2_transaction;

int image2_begin(image2_transaction *, const image2_layout *, const image2_callbacks *, uint32_t);
int image2_write(image2_transaction *, const void *, size_t);
int image2_verify(image2_transaction *);
/* Target signature only. Returns 1 for verified publication, NOT boot switch.
 * No source demotion is allowed. In reviewed normal selection, A->B with both
 * new selects B; B->A with both new STILL selects B. No atomicity, rollback or
 * crash recovery is promised. Power loss/partial writes have uncertain outcome.
 * Even an invalid/out-of-order commit call latches the object forever.
 */
int image2_commit(image2_transaction *);
/* Idempotent, no flash I/O. Never activates or erases a candidate. After commit
 * attempt it cannot undo activation and leaves the permanent latch untouched.
 */
void image2_abort(image2_transaction *);

#ifdef __cplusplus
}
#endif
#endif

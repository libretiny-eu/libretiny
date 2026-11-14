/* Experimental normal-selector coordinator, not an OTA backend or rollback. */
#ifndef AMB1_IMAGE2_ACTIVATION_H
#define AMB1_IMAGE2_ACTIVATION_H

#include "image2_transaction.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Internal coordinator owned by image2_session, not an application API.
 * One instance per boot, never copied/reset/replaced. The session hides this
 * object and its raw transaction callbacks, retains a shared boot latch, and
 * serializes access via the backend. Arbitrary C memory access is not sandboxed.
 */
typedef struct {
	image2_transaction transaction;
	unsigned activation_attempted;
	unsigned trusted_running_slot;
	uint8_t expected_source_signature[8];
} image2_activation;

/* trusted_running_slot is a reliable boot handoff attestation, NOT inferred
 * from flash signatures or the shared SRAM/SDRAM execution address. It must
 * match layout.running_slot. Explicit expected signature must be "81958711".
 * Only the reviewed SDK normal selector profile is supported (B wins ties).
 * Unsupported GPIO/inverted selection, unknown layout or source => refuse.
 * Staging/authentication/validation are delegated to the existing engine.
 */
int image2_activation_begin(
	image2_activation *,
	const image2_layout *,
	const image2_callbacks *,
	uint32_t size,
	unsigned trusted_running_slot,
	const uint8_t expected_source_signature[8]
);
int image2_activation_write(image2_activation *, const void *, size_t);
int image2_activation_verify(image2_activation *);
/* Permanently latches BEFORE validation or any activation read, even on error.
 * Recheck source -> transaction commit (target write + exact readback) ->
 * dedicated backend demote (rechecks target/source, one-byte NOR program,
 * full source signature readback). Generic write NEVER accepts source writes.
 * No erase. Success means target new/source old observed, NOT successful boot.
 * Embedded transaction.state == IMAGE2_COMMITTED describes target publication
 * ONLY; it can coexist with coordinator failure during source demotion.
 * Failure may already select target: A->B switches when target becomes new;
 * B->A switches only when B ceases being new. Torn target signatures may be
 * invalid. No atomicity, retry, automatic rollback or power-loss recovery.
 */
/* demote is a backend-owned, address/data-free capability. */
int image2_activation_commit(image2_activation *, int (*demote)(void *), void *);
void image2_activation_abort(image2_activation *);

#ifdef __cplusplus
}
#endif
#endif

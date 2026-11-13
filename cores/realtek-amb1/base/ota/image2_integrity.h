/* Readback-consistency (integrity) crypto provider for the AMB1 image2 path.
 *
 * INTEGRITY ONLY - NOT AUTHENTICATION.
 *
 * This provider exists because the UF2/ESPHome upload path carries no
 * WCFOTA01 envelope, so there is no authenticated expected digest to compare
 * against. It implements the image2 two-pass contract as a consistency check:
 *
 *   pass 1: SHA-256 over the bytes as received over the wire;
 *   pass 2: SHA-256 over the bytes read back from the inactive slot (with the
 *           boot signature patched to "81958711", exactly like the native
 *           image2 transaction does);
 *   hash_check on pass 2 compares the two digests in constant time.
 *
 * A match proves the staged flash contains the bytes that were received: it
 * detects program/erase/read errors, torn or partially-erased sectors and a
 * corrupted readback. It does NOT prove the image came from a trusted party.
 * There is no key, no MAC, no envelope and no board identity here, so anyone
 * who can deliver a UF2 stream can stage an image. This must never be used
 * where authenticity is required and is explicitly NOT a substitute for the
 * WCFOTA01 HMAC path in image2_auth.c.
 */
#ifndef AMB1_IMAGE2_INTEGRITY_H
#define AMB1_IMAGE2_INTEGRITY_H

#include "image2_session.h"
#include "mbedtls/sha256.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize with {0}. One context per staged image; retain it until the
 * session's terminal call, then cleanup. Never copy a live context, never call
 * cleanup during a session, and do not access it concurrently. Callbacks
 * reject copied contexts and out-of-order use; this is an ownership contract,
 * not protection against arbitrary memory writes. */
typedef struct image2_integrity {
	const struct image2_integrity *identity;
	mbedtls_sha256_context sha;
	/* Exact number of bytes each pass must hash. */
	uint32_t size;
	uint32_t hashed;
	/* Pass-1 (received) digest, compared against pass 2 (readback). */
	uint8_t received_digest[32];
	int authorized, hashing, pass, failed;
} image2_integrity;

/* Record the exact image length that both hash passes will cover. Non-zero
 * only; there is no empty image. Returns 1 on success, 0 on invalid input or a
 * context that has already been initialized. */
int image2_integrity_init(image2_integrity *state, uint32_t size);

/* Fill a session crypto vtable pointing at this context. Returns 1 on success.
 * authorize(): once, the session-provided size must equal the recorded size.
 * hash_reset(): pass 1, then pass 2 only after pass 1 was checked.
 * hash_update(): bounded so a pass can never exceed the recorded size.
 * hash_check(): pass 1 stores the received digest; pass 2 requires an exact
 * length match and a constant-time digest match against pass 1. */
int image2_integrity_callbacks(image2_integrity *state, image2_session_crypto *out);

/* Release the SHA-256 context and zeroize all state. Idempotent; never call
 * during a live session (the session must reach a terminal call first). */
void image2_integrity_cleanup(image2_integrity *state);

#ifdef __cplusplus
}
#endif
#endif

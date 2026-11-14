/* Experimental injected policy only: no production SDK binding. */
#ifndef AMB1_IMAGE2_SESSION_H
#define AMB1_IMAGE2_SESSION_H
#include "image2_activation.h"
#include "image2_flash_backend.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void *ctx;
	int (*authorize)(void *, uint32_t);
	int (*hash_reset)(void *);
	int (*hash_update)(void *, const void *, size_t);
	int (*hash_check)(void *);
} image2_session_crypto;

typedef enum {
	IMAGE2_SESSION_UNUSED,
	IMAGE2_SESSION_STAGING,
	IMAGE2_SESSION_VERIFIED,
	IMAGE2_SESSION_NOT_ACTIVATED,
	IMAGE2_SESSION_ACTIVATED,
	IMAGE2_SESSION_UNCERTAIN
} image2_session_status;

/* Initialize both with {0}. Retain ONE boot object per chip for the ENTIRE
 * boot; all clients share it. Never reset/copy/edit either object. The typed
 * members below are implementation details; callers must not access them.
 * Typed storage avoids effective-type violations without allocation and works
 * in C and C++. This trusted ownership API is NOT a security boundary; it does
 * not hide raw callbacks from code that disregards the ownership contract.
 * All flash users must honor the injected chip lock; no callback reentry.
 * Config, trusted boot identity and crypto policy are snapshotted at begin;
 * external crypto context must remain immutable and live until terminal.
 * A new session cannot bypass the shared boot latch. No retry/reset API.
 */
typedef struct {
	image2_flash_backend flash;
	image2_activation activation;
	image2_session_crypto crypto;
	int used;
} image2_session_boot;

typedef struct image2_session {
	const struct image2_session *identity;
	image2_session_boot *boot;
	image2_session_status status;
	int busy;
} image2_session;

int image2_session_begin(
	image2_session *,
	image2_session_boot *,
	const image2_flash_config *,
	const image2_flash_sdkops *,
	const image2_session_crypto *,
	unsigned trusted_running_slot
);
int image2_session_write(image2_session *, const void *, size_t);
int image2_session_verify(image2_session *);
int image2_session_activate(image2_session *);
void image2_session_abort(image2_session *);
image2_session_status image2_session_get_status(const image2_session *);

/* Terminal calls release the lock exactly once, including failed begin after
 * acquisition. NOT_ACTIVATED means no publication was attempted, NOT a promise
 * that the old image remains selected under power loss or broken callbacks.
 * UNCERTAIN includes any failed activation attempt: callbacks can have physical
 * effects even on error. ACTIVATED means signatures observed, not boot tested.
 * Crypto resources are caller-owned; their cleanup must be idempotent.
 */
#ifdef __cplusplus
}
#endif
#endif

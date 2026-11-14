/* Opt-in WCFOTA01 authentication; no key provisioning or transport binding. */
#ifndef AMB1_IMAGE2_AUTH_H
#define AMB1_IMAGE2_AUTH_H

#include "image2_session.h"
#include "mbedtls/sha256.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IMAGE2_AUTH_HEADER_SIZE	  48u
#define IMAGE2_AUTH_ENVELOPE_SIZE 80u

typedef struct {
	uint32_t min_image_size;
	uint32_t max_image_size;
	/* Zero accepts any authenticated board ID; otherwise require board_id. */
	int require_board_id;
	uint32_t board_id;
} image2_auth_policy;

/* Initialize with {0}; do not copy, modify, or concurrently access a live
 * context. Metadata is copied and authenticated at init; envelope, policy and
 * key need not remain live afterward. The key is caller-owned and is never
 * retained; its owner must erase it when appropriate. No device defaults.
 * Keep this context live through the session's terminal call, then cleanup.
 * Cleanup explicitly erases all state and is idempotent; never call it during
 * a session. Callbacks reject copied contexts and out-of-order use. This is an
 * ownership contract, not protection against arbitrary memory writes.
 */
typedef struct image2_auth {
	const struct image2_auth *identity;
	mbedtls_sha256_context sha;
	uint8_t header[IMAGE2_AUTH_HEADER_SIZE];
	uint32_t image_size;
	uint32_t hashed;
	int authorized;
	int hashing;
	int failed;
} image2_auth;

/* total_size includes the entire 80-byte envelope; exact lengths required.
 * WCFOTA01's version is encoded in its eight-byte magic. HMAC-SHA256 covers
 * bytes [0,48); bytes [48,80) are the MAC. The image digest is [16,48).
 * A successful init authenticates metadata only, not the image payload.
 */
int image2_auth_init(
	image2_auth *,
	const void *envelope,
	size_t envelope_size,
	uint64_t total_size,
	const void *key,
	size_t key_size,
	const image2_auth_policy *
);
int image2_auth_callbacks(image2_auth *, image2_session_crypto *);
void image2_auth_cleanup(image2_auth *);

#ifdef __cplusplus
}
#endif
#endif

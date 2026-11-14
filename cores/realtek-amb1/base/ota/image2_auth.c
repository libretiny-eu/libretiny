#include "image2_auth.h"
#include <string.h>

static void erase(void *p, size_t n) {
	volatile unsigned char *v = p;
	while (n--)
		*v++ = 0;
}

static uint32_t le32(const uint8_t *p) {
	return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static int equal(const uint8_t *a, const uint8_t *b, size_t n) {
	unsigned diff = 0;
	while (n--)
		diff |= *a++ ^ *b++;
	return diff == 0;
}

/* RFC 2104 over the SDK SHA-256 API avoids md's heap-allocated HMAC state.
 * All buffers have fixed bounds; no key or key-derived state survives init.
 */
static void hmac(const void *key, size_t key_size, const uint8_t *header, uint8_t out[32]) {
	mbedtls_sha256_context sha;
	uint8_t pad[64] = {0}, inner[32];
	size_t i;
	mbedtls_sha256_init(&sha);
	if (key_size > sizeof(pad)) {
		mbedtls_sha256_starts(&sha, 0);
		mbedtls_sha256_update(&sha, key, key_size);
		mbedtls_sha256_finish(&sha, pad);
	} else {
		memcpy(pad, key, key_size);
	}
	for (i = 0; i < sizeof(pad); i++)
		pad[i] ^= 0x36;
	mbedtls_sha256_starts(&sha, 0);
	mbedtls_sha256_update(&sha, pad, sizeof(pad));
	mbedtls_sha256_update(&sha, header, IMAGE2_AUTH_HEADER_SIZE);
	mbedtls_sha256_finish(&sha, inner);
	for (i = 0; i < sizeof(pad); i++)
		pad[i] ^= 0x36 ^ 0x5c;
	mbedtls_sha256_starts(&sha, 0);
	mbedtls_sha256_update(&sha, pad, sizeof(pad));
	mbedtls_sha256_update(&sha, inner, sizeof(inner));
	mbedtls_sha256_finish(&sha, out);
	mbedtls_sha256_free(&sha);
	erase(&sha, sizeof(sha));
	erase(pad, sizeof(pad));
	erase(inner, sizeof(inner));
}

void image2_auth_cleanup(image2_auth *a) {
	if (!a)
		return;
	/* Context uses SDK SHA-256 fixed storage, never md heap ownership. */
	if (a->identity == a)
		mbedtls_sha256_free(&a->sha);
	erase(a, sizeof(*a));
}

int image2_auth_init(
	image2_auth *a,
	const void *envelope,
	size_t envelope_size,
	uint64_t total_size,
	const void *key,
	size_t key_size,
	const image2_auth_policy *policy
) {
	uint8_t frozen[IMAGE2_AUTH_ENVELOPE_SIZE], mac[32];
	uint32_t size;
	int ok;
	if (!a || a->identity || !envelope || !key || !key_size || !policy || envelope_size != IMAGE2_AUTH_ENVELOPE_SIZE ||
		!policy->min_image_size || policy->min_image_size > policy->max_image_size)
		return 0;
	memcpy(frozen, envelope, sizeof(frozen));
	size = le32(frozen + 8);
	ok	 = !memcmp(frozen, "WCFOTA01", 8) && size >= policy->min_image_size && size <= policy->max_image_size &&
		   total_size == (uint64_t)size + IMAGE2_AUTH_ENVELOPE_SIZE &&
		   (!policy->require_board_id || le32(frozen + 12) == policy->board_id);
	if (ok) {
		hmac(key, key_size, frozen, mac);
		ok = equal(mac, frozen + IMAGE2_AUTH_HEADER_SIZE, sizeof(mac));
	}
	if (ok) {
		erase(a, sizeof(*a));
		mbedtls_sha256_init(&a->sha);
		memcpy(a->header, frozen, sizeof(a->header));
		a->image_size = size;
		a->identity	  = a;
	}
	erase(mac, sizeof(mac));
	erase(frozen, sizeof(frozen));
	return ok;
}

static image2_auth *valid(void *p) {
	image2_auth *a = p;
	return a && a->identity == a && !a->failed ? a : NULL;
}

static int authorize(void *p, uint32_t size) {
	image2_auth *a = valid(p);
	if (!a)
		return 0;
	if (a->authorized || size != a->image_size) {
		a->failed = 1;
		return 0;
	}
	a->authorized = 1;
	return 1;
}

static int hash_reset(void *p) {
	image2_auth *a = valid(p);
	if (!a)
		return 0;
	if (!a->authorized || a->hashing) {
		a->failed = 1;
		return 0;
	}
	mbedtls_sha256_starts(&a->sha, 0);
	a->hashed  = 0;
	a->hashing = 1;
	return 1;
}

static int hash_update(void *p, const void *data, size_t size) {
	image2_auth *a = valid(p);
	if (!a)
		return 0;
	if (!a->hashing || (!data && size) || size > a->image_size - a->hashed) {
		a->failed = 1;
		return 0;
	}
	if (size)
		mbedtls_sha256_update(&a->sha, data, size);
	a->hashed += (uint32_t)size;
	return 1;
}

static int hash_check(void *p) {
	image2_auth *a = valid(p);
	uint8_t digest[32];
	int ok;
	if (!a)
		return 0;
	if (!a->hashing || a->hashed != a->image_size) {
		a->failed = 1;
		return 0;
	}
	mbedtls_sha256_finish(&a->sha, digest);
	ok = equal(digest, a->header + 16, sizeof(digest));
	erase(digest, sizeof(digest));
	a->hashing = 0;
	a->failed  = !ok;
	return ok;
}

int image2_auth_callbacks(image2_auth *a, image2_session_crypto *out) {
	if (!out)
		return 0;
	memset(out, 0, sizeof(*out));
	if (!valid(a))
		return 0;
	out->ctx		 = a;
	out->authorize	 = authorize;
	out->hash_reset	 = hash_reset;
	out->hash_update = hash_update;
	out->hash_check	 = hash_check;
	return 1;
}

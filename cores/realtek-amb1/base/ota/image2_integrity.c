/* Readback-consistency (integrity) provider. See image2_integrity.h: this is
 * explicitly NOT authentication and carries no key or envelope. */
#include "image2_integrity.h"
#include <string.h>

/* Pass state machine. `pass` doubles as the ordering latch so an out-of-order
 * reset/check can never silently reuse a digest. */
enum {
	PASS_IDLE	  = 0, /* nothing hashed yet */
	PASS_RECEIVED = 1, /* hashing pass 1 (received bytes) */
	PASS_READY	  = 2, /* pass 1 checked, received_digest is valid */
	PASS_READBACK = 3, /* hashing pass 2 (readback bytes) */
	PASS_DONE	  = 4, /* pass 2 checked; no further use */
};

static void erase(void *p, size_t n) {
	volatile unsigned char *v = p;
	while (n--)
		*v++ = 0;
}

/* Mirrors the constant-time comparison in image2_auth.c; a shared static helper
 * cannot be linked across translation units without changing that module. */
static int equal(const uint8_t *a, const uint8_t *b, size_t n) {
	unsigned diff = 0;
	while (n--)
		diff |= *a++ ^ *b++;
	return diff == 0;
}

static image2_integrity *valid(void *p) {
	image2_integrity *s = p;
	return s && s->identity == s && !s->failed ? s : NULL;
}

int image2_integrity_init(image2_integrity *s, uint32_t size) {
	if (!s || s->identity || !size)
		return 0;
	erase(s, sizeof(*s));
	mbedtls_sha256_init(&s->sha);
	s->size		= size;
	s->identity = s;
	return 1;
}

static int authorize(void *p, uint32_t size) {
	image2_integrity *s = valid(p);
	if (!s)
		return 0;
	if (s->authorized || size != s->size) {
		s->failed = 1;
		return 0;
	}
	s->authorized = 1;
	return 1;
}

static int hash_reset(void *p) {
	image2_integrity *s = valid(p);
	if (!s)
		return 0;
	if (!s->authorized || s->hashing || s->pass == PASS_DONE || (s->pass != PASS_IDLE && s->pass != PASS_READY)) {
		s->failed = 1;
		return 0;
	}
	mbedtls_sha256_starts(&s->sha, 0);
	s->hashed  = 0;
	s->hashing = 1;
	s->pass	   = s->pass == PASS_IDLE ? PASS_RECEIVED : PASS_READBACK;
	return 1;
}

static int hash_update(void *p, const void *data, size_t size) {
	image2_integrity *s = valid(p);
	if (!s)
		return 0;
	if (!s->hashing || (!data && size) || s->hashed > s->size || size > s->size - s->hashed) {
		s->failed = 1;
		return 0;
	}
	if (size)
		mbedtls_sha256_update(&s->sha, data, size);
	s->hashed += (uint32_t)size;
	return 1;
}

static int hash_check(void *p) {
	image2_integrity *s = valid(p);
	uint8_t digest[32];
	int ok;
	if (!s)
		return 0;
	if (!s->hashing || s->hashed != s->size || (s->pass != PASS_RECEIVED && s->pass != PASS_READBACK)) {
		s->failed = 1;
		return 0;
	}
	mbedtls_sha256_finish(&s->sha, digest);
	if (s->pass == PASS_RECEIVED) {
		memcpy(s->received_digest, digest, sizeof(s->received_digest));
		ok		= 1;
		s->pass = PASS_READY;
	} else {
		ok		= equal(digest, s->received_digest, sizeof(digest));
		s->pass = PASS_DONE;
	}
	erase(digest, sizeof(digest));
	s->hashing = 0;
	s->failed  = !ok;
	return ok;
}

int image2_integrity_callbacks(image2_integrity *s, image2_session_crypto *out) {
	if (!out)
		return 0;
	memset(out, 0, sizeof(*out));
	/* authorize() runs later, inside image2_begin(). */
	if (!valid(s))
		return 0;
	out->ctx		 = s;
	out->authorize	 = authorize;
	out->hash_reset	 = hash_reset;
	out->hash_update = hash_update;
	out->hash_check	 = hash_check;
	return 1;
}

void image2_integrity_cleanup(image2_integrity *s) {
	if (!s)
		return;
	if (s->identity == s)
		mbedtls_sha256_free(&s->sha);
	erase(s, sizeof(*s));
}

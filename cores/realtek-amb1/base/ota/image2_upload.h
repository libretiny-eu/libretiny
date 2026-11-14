/* Experimental native WCFOTA01 upload, NOT UF2 or an enabled Update binding.
 * No HTTP protocol, SDK binding, embedded key, or automatic reboot. */
#ifndef AMB1_IMAGE2_UPLOAD_H
#define AMB1_IMAGE2_UPLOAD_H
#include "image2_auth.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Initialize with {0}. Keep ONE boot object per chip for the entire boot,
 * shared by all clients, including failed/unauthenticated clients. Never reset
 * or copy live objects; serialize calls and honor the session chip lock.
 * Key storage is borrowed until the envelope arrives or abort/finish returns;
 * its owner must erase it. Other inputs are snapshotted at begin. */
typedef struct {
	image2_session_boot session;
	int used;
} image2_upload_boot;

typedef struct image2_upload {
	const struct image2_upload *identity;
	image2_upload_boot *boot;
	image2_session session;
	image2_auth auth;
	image2_auth_policy policy;
	image2_flash_config config;
	image2_flash_sdkops ops;
	const void *key;
	size_t key_size;
	uint64_t total, received;
	uint8_t envelope[IMAGE2_AUTH_ENVELOPE_SIZE];
	unsigned running;
	int active, busy;
} image2_upload;

int image2_upload_begin(
	image2_upload *,
	image2_upload_boot *,
	uint64_t total,
	const image2_flash_config *,
	const image2_flash_sdkops *,
	unsigned running,
	const image2_auth_policy *,
	const void *key,
	size_t key_size
);
int image2_upload_write(image2_upload *, const void *, size_t);
/* Requires exact total and verified readback. activate must be 0 or 1;
 * zero verifies then closes without publishing. Query session status for
 * activation uncertainty. Every terminal path erases authentication state. */
int image2_upload_finish(image2_upload *, int activate);
void image2_upload_abort(image2_upload *);
#ifdef __cplusplus
}
#endif
#endif

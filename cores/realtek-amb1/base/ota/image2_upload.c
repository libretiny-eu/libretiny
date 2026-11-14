#include "image2_upload.h"
#include <string.h>

static int live(image2_upload *u) {
	return u && u->identity == u && u->active && !u->busy;
}

static void erase_auth(image2_upload *u) {
	volatile uint8_t *p = u->envelope;
	size_t n			= sizeof(u->envelope);
	image2_auth_cleanup(&u->auth);
	while (n--)
		*p++ = 0;
	u->key		= NULL;
	u->key_size = 0;
}

static int terminal(image2_upload *u, int ok) {
	image2_session_abort(&u->session);
	erase_auth(u);
	u->active = u->busy = 0;
	return ok;
}

int image2_upload_begin(
	image2_upload *u,
	image2_upload_boot *b,
	uint64_t total,
	const image2_flash_config *config,
	const image2_flash_sdkops *ops,
	unsigned running,
	const image2_auth_policy *policy,
	const void *key,
	size_t key_size
) {
	if (!u || !b || u->identity || b->used || b->session.used)
		return 0;
	b->used		= 1;
	u->identity = u;
	u->boot		= b;
	u->active = u->busy = 1;
	if (!config || !ops || !policy || !key || !key_size || running > 1 || total <= IMAGE2_AUTH_ENVELOPE_SIZE ||
		total - IMAGE2_AUTH_ENVELOPE_SIZE > UINT32_MAX || total - IMAGE2_AUTH_ENVELOPE_SIZE != config->image_size)
		return terminal(u, 0);
	u->config	= *config;
	u->ops		= *ops;
	u->policy	= *policy;
	u->running	= running;
	u->total	= total;
	u->key		= key;
	u->key_size = key_size;
	u->busy		= 0;
	return 1;
}

int image2_upload_write(image2_upload *u, const void *data, size_t n) {
	const uint8_t *p = data;
	image2_session_crypto crypto;
	size_t take;
	if (!live(u))
		return 0;
	u->busy = 1;
	if ((!p && n) || n > u->total - u->received)
		return terminal(u, 0);
	if (u->received < IMAGE2_AUTH_ENVELOPE_SIZE) {
		take = IMAGE2_AUTH_ENVELOPE_SIZE - (size_t)u->received;
		if (take > n)
			take = n;
		if (take)
			memcpy(u->envelope + (size_t)u->received, p, take);
		u->received += take;
		n -= take;
		if (take)
			p += take;
		if (u->received == IMAGE2_AUTH_ENVELOPE_SIZE) {
			if (!image2_auth_init(
					&u->auth,
					u->envelope,
					sizeof(u->envelope),
					u->total,
					u->key,
					u->key_size,
					&u->policy
				) ||
				!image2_auth_callbacks(&u->auth, &crypto))
				return terminal(u, 0);
			u->key		= NULL;
			u->key_size = 0;
			if (!image2_session_begin(&u->session, &u->boot->session, &u->config, &u->ops, &crypto, u->running))
				return terminal(u, 0);
		}
	}
	if (n && !image2_session_write(&u->session, p, n))
		return terminal(u, 0);
	u->received += n;
	u->busy = 0;
	return 1;
}

int image2_upload_finish(image2_upload *u, int activate) {
	int ok;
	if (!live(u))
		return 0;
	u->busy = 1;
	if ((activate != 0 && activate != 1) || u->received != u->total || !image2_session_verify(&u->session))
		return terminal(u, 0);
	ok = !activate || image2_session_activate(&u->session);
	return terminal(u, ok);
}

void image2_upload_abort(image2_upload *u) {
	if (live(u)) {
		u->busy = 1;
		(void)terminal(u, 0);
	}
}

#include "image2_session.h"
#include "image2_activation.h"
#include <string.h>

typedef image2_session_boot boot_state;
typedef image2_session session_state;

static int read_flash(void *p, uint32_t a, void *v, size_t n) {
	return image2_flash_read(&((boot_state *)p)->flash,a,v,n);
}
static int write_flash(void *p, uint32_t a, const void *v, size_t n) {
	return image2_flash_write(&((boot_state *)p)->flash,a,v,n);
}
static int erase_flash(void *p, uint32_t a, size_t n) {
	return image2_flash_erase(&((boot_state *)p)->flash,a,n);
}
static int authorize(void *p, uint32_t n) {
	boot_state *b = p; return b->crypto.authorize(b->crypto.ctx,n) == 1;
}
static int hash_reset(void *p) {
	boot_state *b = p; return b->crypto.hash_reset(b->crypto.ctx) == 1;
}
static int hash_update(void *p, const void *v, size_t n) {
	boot_state *b = p; return b->crypto.hash_update(b->crypto.ctx,v,n) == 1;
}
static int hash_check(void *p) {
	boot_state *b = p; return b->crypto.hash_check(b->crypto.ctx) == 1;
}
static int demote(void *p) { return image2_flash_demote(&((boot_state *)p)->flash); }
static int finish(session_state *s, image2_session_status status, int result) {
	s->status = status;
	image2_flash_close(&s->boot->flash);
	s->busy = 0;
	return result;
}
static session_state *state(image2_session *s) {
	return s && s->identity == s ? s : NULL;
}

int image2_session_begin(image2_session *session, image2_session_boot *boot,
	const image2_flash_config *config, const image2_flash_sdkops *ops,
	const image2_session_crypto *crypto, unsigned running) {
	session_state *s = session;
	boot_state *b = boot;
	image2_callbacks io;
	if (!s || !b || s->identity || b->used) return 0;
	b->used = 1; s->identity = session; s->boot = b; s->busy = 1;
	s->status = IMAGE2_SESSION_NOT_ACTIVATED;
	if (!config || !crypto || !crypto->authorize || !crypto->hash_reset ||
	    !crypto->hash_update || !crypto->hash_check || running > 1 ||
	    running != config->layout.running_slot || !config->allow_source_demote)
		return finish(s,IMAGE2_SESSION_NOT_ACTIVATED,0);
	b->crypto = *crypto;
	if (!image2_flash_open(&b->flash,config,ops))
		return finish(s,IMAGE2_SESSION_NOT_ACTIVATED,0);
	io.ctx = b; io.read = read_flash; io.write = write_flash; io.erase = erase_flash;
	io.authorize = authorize; io.hash_reset = hash_reset;
	io.hash_update = hash_update; io.hash_check = hash_check;
	if (!image2_activation_begin(&b->activation,&b->flash.config.layout,&io,
	    b->flash.config.image_size,running,(const uint8_t *)"81958711"))
		return finish(s,IMAGE2_SESSION_NOT_ACTIVATED,0);
	s->status = IMAGE2_SESSION_STAGING; s->busy = 0;
	return 1;
}

int image2_session_write(image2_session *session, const void *p, size_t n) {
	session_state *s = state(session);
	if (!s || s->busy) return 0;
	if (s->status == IMAGE2_SESSION_VERIFIED)
		return finish(s,IMAGE2_SESSION_NOT_ACTIVATED,0);
	if (s->status != IMAGE2_SESSION_STAGING) return 0;
	s->busy = 1;
	if (!image2_activation_write(&s->boot->activation,p,n))
		return finish(s,IMAGE2_SESSION_NOT_ACTIVATED,0);
	s->busy = 0; return 1;
}

int image2_session_verify(image2_session *session) {
	session_state *s = state(session);
	if (!s || s->busy) return 0;
	if (s->status == IMAGE2_SESSION_VERIFIED)
		return finish(s,IMAGE2_SESSION_NOT_ACTIVATED,0);
	if (s->status != IMAGE2_SESSION_STAGING) return 0;
	s->busy = 1;
	if (!image2_flash_phase(&s->boot->flash,2) ||
	    !image2_activation_verify(&s->boot->activation))
		return finish(s,IMAGE2_SESSION_NOT_ACTIVATED,0);
	s->status = IMAGE2_SESSION_VERIFIED; s->busy = 0; return 1;
}

int image2_session_activate(image2_session *session) {
	session_state *s = state(session);
	int ok;
	if (!s || s->busy || (s->status != IMAGE2_SESSION_STAGING &&
	    s->status != IMAGE2_SESSION_VERIFIED)) return 0;
	s->busy = 1;
	if (s->status != IMAGE2_SESSION_VERIFIED)
		return finish(s,IMAGE2_SESSION_NOT_ACTIVATED,0);
	s->status = IMAGE2_SESSION_UNCERTAIN;
	ok = image2_flash_phase(&s->boot->flash,3) &&
	     image2_activation_commit(&s->boot->activation,demote,s->boot);
	return finish(s,ok ? IMAGE2_SESSION_ACTIVATED : IMAGE2_SESSION_UNCERTAIN,ok);
}

void image2_session_abort(image2_session *session) {
	session_state *s = state(session);
	if (!s || s->busy || (s->status != IMAGE2_SESSION_STAGING &&
	    s->status != IMAGE2_SESSION_VERIFIED)) return;
	s->busy = 1; image2_activation_abort(&s->boot->activation);
	(void)finish(s,IMAGE2_SESSION_NOT_ACTIVATED,0);
}

image2_session_status image2_session_get_status(const image2_session *session) {
	return session && session->identity == session ? session->status : IMAGE2_SESSION_UNUSED;
}

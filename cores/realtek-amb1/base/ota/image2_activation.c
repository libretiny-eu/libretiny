#include "image2_activation.h"
#include <string.h>

static const uint8_t new_signature[8] = {'8','1','9','5','8','7','1','1'};

int image2_activation_begin(image2_activation *a, const image2_layout *l,
	const image2_callbacks *io, uint32_t size, unsigned running,
	const uint8_t expected[8]) {
	if (!a || a->activation_attempted || a->transaction.state != IMAGE2_IDLE) return 0;
	if (!l || running > 1 || running != l->running_slot || !expected ||
	    memcmp(expected,new_signature,8)) {
		image2_abort(&a->transaction);
		return 0;
	}
	a->trusted_running_slot = running;
	memcpy(a->expected_source_signature,expected,8);
	return image2_begin(&a->transaction,l,io,size);
}

int image2_activation_write(image2_activation *a, const void *p, size_t n) {
	return a && !a->activation_attempted && image2_write(&a->transaction,p,n);
}

int image2_activation_verify(image2_activation *a) {
	return a && !a->activation_attempted && image2_verify(&a->transaction);
}

int image2_activation_commit(image2_activation *a, int (*demote)(void *), void *ctx) {
	image2_transaction *t;
	uint32_t source;
	uint8_t sig[8];
	if (!a || a->activation_attempted) return 0;
	a->activation_attempted = 1;
	t = &a->transaction;
	if (!demote || t->state != IMAGE2_VERIFIED || a->trusted_running_slot > 1 ||
	    t->layout.running_slot != a->trusted_running_slot ||
	    !t->layout.reviewed_layout || !t->layout.normal_selector ||
	    memcmp(a->expected_source_signature,new_signature,8)) {
		image2_abort(t);
		return 0;
	}
	source = t->layout.slots[a->trusted_running_slot].start + 8;
	if (t->io.read(t->io.ctx,source,sig,8) != 1 ||
	    memcmp(sig,a->expected_source_signature,8)) {
		image2_abort(t);
		return 0;
	}
	if (!image2_commit(t)) return 0;
	return demote(ctx) == 1;
}

void image2_activation_abort(image2_activation *a) {
	if (a) image2_abort(&a->transaction);
}

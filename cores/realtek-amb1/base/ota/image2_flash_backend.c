#include "image2_flash_backend.h"
#include <string.h>

#define CHECK_CHUNK 128u

static int valid_range(image2_range r, uint32_t size) {
	return r.start < r.end && r.end <= size;
}

static int overlap(image2_range a, image2_range b) {
	return a.start < b.end && b.start < a.end;
}

static int contains(image2_range r, uint32_t a, size_t n) {
	return n && a >= r.start && a < r.end && n <= r.end - a;
}

static int geometry(const image2_flash_config *c) {
	const image2_layout *l = &c->layout;
	image2_range active, target;
	uint32_t rounded;
	size_t i;
	if (!l->reviewed_layout || !l->normal_selector || l->running_slot > 1 || !l->sector_size || !c->page_size ||
		l->sector_size % c->page_size || l->protected_count > IMAGE2_MAX_PROTECTED || c->image_size < 42 ||
		c->image_size > UINT32_MAX - (l->sector_size - 1))
		return 0;
	active = l->slots[l->running_slot];
	target = l->slots[1 - l->running_slot];
	if (!valid_range(active, l->flash_size) || !valid_range(target, l->flash_size) || active.end - active.start < 16 ||
		overlap(active, target) || !valid_range(l->reviewed, l->flash_size) || target.start % l->sector_size ||
		target.end % l->sector_size || l->reviewed.start != target.start || l->reviewed.end > target.end ||
		l->reviewed.end % l->sector_size)
		return 0;
	rounded = ((c->image_size + l->sector_size - 1) / l->sector_size) * l->sector_size;
	if (rounded > l->reviewed.end - target.start || c->extent.start != target.start ||
		c->extent.end != target.start + rounded)
		return 0;
	for (i = 0; i < l->protected_count; i++)
		if (!valid_range(l->protected_ranges[i], l->flash_size) || overlap(l->reviewed, l->protected_ranges[i]))
			return 0;
	return 1;
}

int image2_flash_open(image2_flash_backend *b, const image2_flash_config *c, const image2_flash_sdkops *ops) {
	if (!b || !c || !ops || !ops->try_lock || !ops->unlock || !ops->read || !ops->program || !ops->erase_sector ||
		b->used)
		return 0;
	b->identity = b;
	b->used		= 1;
	b->ops		= *ops;
	if (ops->try_lock(ops->ctx, b) != 1)
		return 0;
	b->held	  = 1;
	b->phase  = 1;
	b->config = *c;
	if (!geometry(c)) {
		b->failed = 1;
		return 0;
	}
	return 1;
}

void image2_flash_close(image2_flash_backend *b) {
	if (!b || b->identity != b || !b->held || b->busy)
		return;
	b->held	  = 0;
	b->failed = 1;
	b->phase  = 4;
	b->ops.unlock(b->ops.ctx, b);
}

static int enter(image2_flash_backend *b) {
	if (!b || b->identity != b || !b->held || b->failed || b->phase == 4)
		return 0;
	if (b->busy) {
		b->failed = 1;
		return 0;
	}
	b->busy = 1;
	return 1;
}

static int leave(image2_flash_backend *b, int ok) {
	if (!ok)
		b->failed = 1;
	b->busy = 0;
	return ok && !b->failed;
}

int image2_flash_read(void *ctx, uint32_t a, void *p, size_t n) {
	image2_flash_backend *b = ctx;
	image2_range sig;
	int ok;
	if (!enter(b))
		return 0;
	sig.start = b->config.layout.slots[b->config.layout.running_slot].start + 8;
	sig.end	  = sig.start + 8;
	ok		  = p && (contains(b->config.extent, a, n) || (a == sig.start && n == 8));
	if (ok)
		ok = b->ops.read(b->ops.ctx, a, p, n) == 1;
	return leave(b, ok);
}

int image2_flash_erase(void *ctx, uint32_t a, size_t n) {
	image2_flash_backend *b = ctx;
	uint8_t check[CHECK_CHUNK];
	size_t off, part, i;
	int ok;
	if (!enter(b))
		return 0;
	ok = b->phase == 1 && n == b->config.layout.sector_size && a % n == 0 && contains(b->config.extent, a, n);
	if (ok)
		ok = b->ops.erase_sector(b->ops.ctx, a) == 1 && !b->failed;
	for (off = 0; ok && off < n; off += part) {
		part = n - off < sizeof(check) ? n - off : sizeof(check);
		ok	 = b->ops.read(b->ops.ctx, a + (uint32_t)off, check, part) == 1 && !b->failed;
		for (i = 0; ok && i < part; i++)
			if (check[i] != 255)
				ok = 0;
	}
	return leave(b, ok);
}

int image2_flash_write(void *ctx, uint32_t a, const void *data, size_t n) {
	image2_flash_backend *b = ctx;
	const uint8_t *p		= data;
	uint8_t check[CHECK_CHUNK];
	size_t off, part, i;
	int ok;
	if (!enter(b))
		return 0;
	ok = p && contains(b->config.extent, a, n) &&
		 (b->phase == 1 || (b->phase == 3 && a == b->config.extent.start + 8 && n == 8 &&
							memcmp(p, "81958711", 8) == 0 && !b->publication_attempted));
	if (ok && b->phase == 3)
		b->publication_attempted = 1;
	/* Preflight the ENTIRE request before programming, including NOR 0->1. */
	for (off = 0; ok && off < n; off += part) {
		part = n - off < sizeof(check) ? n - off : sizeof(check);
		ok	 = b->ops.read(b->ops.ctx, a + (uint32_t)off, check, part) == 1 && !b->failed;
		for (i = 0; ok && i < part; i++)
			if ((check[i] & p[off + i]) != p[off + i])
				ok = 0;
	}
	for (off = 0; ok && off < n; off += part) {
		uint32_t address = a + (uint32_t)off;
		part			 = b->config.page_size - address % b->config.page_size;
		if (part > sizeof(check))
			part = sizeof(check);
		if (part > n - off)
			part = n - off;
		ok = b->ops.program(b->ops.ctx, address, p + off, part) == 1 && !b->failed;
	}
	/* Compare every byte, not just the final program fragment or signature. */
	for (off = 0; ok && off < n; off += part) {
		part = n - off < sizeof(check) ? n - off : sizeof(check);
		ok	 = b->ops.read(b->ops.ctx, a + (uint32_t)off, check, part) == 1 && !b->failed &&
			   memcmp(check, p + off, part) == 0;
	}
	return leave(b, ok);
}

int image2_flash_phase(image2_flash_backend *b, int phase) {
	if (!b || b->identity != b || !b->held || b->failed || b->busy || phase != b->phase + 1 || phase > 3)
		return 0;
	b->phase = phase;
	return 1;
}

int image2_flash_demote(image2_flash_backend *b) {
	uint8_t sig[8], old = '0';
	uint32_t source, target;
	int ok;
	if (!enter(b))
		return 0;
	source = b->config.layout.slots[b->config.layout.running_slot].start + 8;
	target = b->config.extent.start + 8;
	ok	   = b->phase == 3 && b->publication_attempted && b->config.allow_source_demote;
	/* Recheck publication and the FULL source signature before the sole
	 * protected-range exception. Never route this through generic write. */
	if (ok)
		ok = b->ops.read(b->ops.ctx, target, sig, 8) == 1 && !memcmp(sig, "81958711", 8) && !b->failed;
	if (ok)
		ok = b->ops.read(b->ops.ctx, source, sig, 8) == 1 && !memcmp(sig, "81958711", 8) && !b->failed;
	/* Prefer the adapter's dedicated demote capability, which is authorized for
	 * exactly this one byte. Only legacy/mock adapters without one fall back to
	 * the generic program path. */
	if (ok && b->ops.demote)
		ok = b->ops.demote(b->ops.ctx) == 1 && !b->failed;
	else if (ok)
		ok = b->ops.program(b->ops.ctx, source, &old, 1) == 1 && !b->failed;
	if (ok)
		ok = b->ops.read(b->ops.ctx, source, sig, 8) == 1 && !memcmp(sig, "01958711", 8) && !b->failed;
	b->phase = 4;
	return leave(b, ok);
}

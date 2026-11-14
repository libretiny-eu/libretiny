#include "image2_transaction.h"
#include <string.h>

static const uint8_t new_signature[8] = {'8', '1', '9', '5', '8', '7', '1', '1'};
static const uint8_t old_signature[8] = {'0', '1', '9', '5', '8', '7', '1', '1'};
static const uint8_t erased[8]		  = {255, 255, 255, 255, 255, 255, 255, 255};

static int fail(image2_transaction *t) {
	t->state = IMAGE2_FAILED;
	return 0;
}

static int range(image2_range r, uint32_t size) {
	return r.start < r.end && r.end <= size;
}

static int overlaps(image2_range a, image2_range b) {
	return a.start < b.end && b.start < a.end;
}

static uint32_t le32(const uint8_t *p) {
	return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static int read_at(image2_transaction *t, uint32_t off, void *p, size_t n) {
	if (off > t->size || n > t->size - off)
		return 0;
	return t->io.read(t->io.ctx, t->target + off, p, n) == 1;
}

static int blank(image2_transaction *t, uint32_t address, uint32_t length) {
	uint8_t b[128];
	while (length) {
		size_t n = length < sizeof(b) ? length : sizeof(b), i;
		if (t->io.read(t->io.ctx, address, b, n) != 1)
			return 0;
		for (i = 0; i < n; i++)
			if (b[i] != 255)
				return 0;
		address += (uint32_t)n;
		length -= (uint32_t)n;
	}
	return 1;
}

int image2_begin(image2_transaction *t, const image2_layout *l, const image2_callbacks *io, uint32_t size) {
	image2_range active, target, extent;
	uint32_t rounded, pos;
	uint8_t sig[8], zero[8] = {0};
	size_t i;
	if (!t || t->state != IMAGE2_IDLE)
		return 0;
	/* A rejected begin also consumes this object: no uncertain retry paths. */
	t->state = IMAGE2_FAILED;
	if (!l || !io || !io->read || !io->erase || !io->write || !io->authorize || !io->hash_reset || !io->hash_update ||
		!io->hash_check || !l->reviewed_layout || !l->normal_selector || l->running_slot > 1 || !l->sector_size ||
		l->protected_count > IMAGE2_MAX_PROTECTED || size < 42 || l->sram_base >= l->sram_end ||
		l->sdram_base >= l->sdram_end)
		return 0;
	active = l->slots[l->running_slot];
	target = l->slots[1 - l->running_slot];
	if (!range(active, l->flash_size) || !range(target, l->flash_size) || !range(l->reviewed, l->flash_size) ||
		overlaps(active, target) || active.end - active.start < 16 || target.end - target.start < 16 ||
		target.start % l->sector_size || target.end % l->sector_size || l->reviewed.start % l->sector_size ||
		l->reviewed.end % l->sector_size || l->reviewed.start != target.start || l->reviewed.end > target.end ||
		size > UINT32_MAX - (l->sector_size - 1))
		return 0;
	rounded = ((size + l->sector_size - 1) / l->sector_size) * l->sector_size;
	if (rounded > l->reviewed.end - target.start)
		return 0;
	extent.start = target.start;
	extent.end	 = target.start + rounded;
	for (i = 0; i < l->protected_count; i++)
		if (!range(l->protected_ranges[i], l->flash_size) || overlaps(l->reviewed, l->protected_ranges[i]))
			return 0;
	t->layout	 = *l;
	t->io		 = *io;
	t->size		 = size;
	t->target	 = extent.start;
	t->erase_end = extent.end;
	if (io->authorize(io->ctx, size) != 1 || io->read(io->ctx, active.start + 8, sig, 8) != 1 ||
		memcmp(sig, new_signature, 8) || io->read(io->ctx, target.start + 8, sig, 8) != 1 ||
		(memcmp(sig, old_signature, 8) && memcmp(sig, erased, 8)) || io->hash_reset(io->ctx) != 1)
		return 0;
	/* Invalidate an old candidate before erasing other sectors. Never erase or
	 * alter the running slot. Failure is terminal; no cleanup mutation follows.
	 */
	if (memcmp(sig, erased, 8) && (io->write(io->ctx, target.start + 8, zero, 8) != 1 ||
								   io->read(io->ctx, target.start + 8, sig, 8) != 1 || memcmp(sig, zero, 8)))
		return 0;
	for (pos = extent.start; pos < extent.end; pos += l->sector_size)
		if (io->erase(io->ctx, pos, l->sector_size) != 1 || !blank(t, pos, l->sector_size))
			return 0;
	t->state = IMAGE2_STAGING;
	return 1;
}

int image2_write(image2_transaction *t, const void *data, size_t n) {
	const uint8_t *p = data;
	if (!t || t->state != IMAGE2_STAGING)
		return 0;
	if ((!data && n) || n > t->size - t->received)
		return fail(t);
	if (!n)
		return 1;
	if (t->io.hash_update(t->io.ctx, p, n) != 1)
		return fail(t);
	while (n) {
		uint32_t off = t->received;
		size_t part	 = n;
		if (off < 8 && part > 8 - off)
			part = 8 - off;
		if (off >= 8 && off < 16) {
			if (part > 16 - off)
				part = 16 - off;
			memcpy(t->signature + off - 8, p, part);
		} else if (t->io.write(t->io.ctx, t->target + off, p, part) != 1)
			return fail(t);
		t->received += (uint32_t)part;
		p += part;
		n -= part;
	}
	return 1;
}

int image2_verify(image2_transaction *t) {
	uint8_t b[128], h[26];
	uint32_t ram, second, dram, entry, off;
	if (!t || t->state != IMAGE2_STAGING)
		return 0;
	if (t->received != t->size || memcmp(t->signature, new_signature, 8) || t->io.hash_check(t->io.ctx) != 1 ||
		!read_at(t, 0, h, sizeof(h)) || memcmp(h + 8, erased, 8))
		return fail(t);
	ram	  = le32(h);
	entry = le32(h + 16);
	if (le32(h + 4) != t->layout.sram_base || ram < 10 || ram > t->layout.sram_end - t->layout.sram_base ||
		ram > t->size - 32 || !(entry & 1) || (entry & ~1u) < t->layout.sram_base ||
		(entry & ~1u) - t->layout.sram_base > ram - 2 || memcmp(h + 20, "RTKWin", 6))
		return fail(t);
	second = 16 + ram;
	if (!read_at(t, second, h, 16))
		return fail(t);
	dram = le32(h);
	if (le32(h + 4) != t->layout.sdram_base || dram > t->layout.sdram_end - t->layout.sdram_base ||
		memcmp(h + 8, erased, 8) || dram != t->size - second - 16 || t->io.hash_reset(t->io.ctx) != 1)
		return fail(t);
	for (off = 0; off < t->size;) {
		size_t n = t->size - off < sizeof(b) ? t->size - off : sizeof(b);
		if (!read_at(t, off, b, n))
			return fail(t);
		if (!off) {
			if (memcmp(b + 8, erased, 8))
				return fail(t);
			memcpy(b + 8, new_signature, 8);
		}
		if (t->io.hash_update(t->io.ctx, b, n) != 1)
			return fail(t);
		off += (uint32_t)n;
	}
	if (t->io.hash_check(t->io.ctx) != 1)
		return fail(t);
	t->state = IMAGE2_VERIFIED;
	return 1;
}

int image2_commit(image2_transaction *t) {
	uint8_t sig[8];
	int ready;
	if (!t)
		return 0;
	ready = t->state == IMAGE2_VERIFIED;
	/* Latch BEFORE any callback, including reads. Errors cannot reopen staging. */
	if (t->state == IMAGE2_COMMIT_ATTEMPTED || t->state == IMAGE2_COMMITTED)
		return 0;
	t->state = IMAGE2_COMMIT_ATTEMPTED;
	if (!ready || !read_at(t, 8, sig, 8) || memcmp(sig, erased, 8) ||
		t->io.write(t->io.ctx, t->target + 8, new_signature, 8) != 1 || !read_at(t, 8, sig, 8) ||
		memcmp(sig, new_signature, 8))
		return 0;
	t->state = IMAGE2_COMMITTED;
	return 1;
}

void image2_abort(image2_transaction *t) {
	if (t && (t->state == IMAGE2_IDLE || t->state == IMAGE2_STAGING || t->state == IMAGE2_VERIFIED))
		t->state = IMAGE2_ABORTED;
}

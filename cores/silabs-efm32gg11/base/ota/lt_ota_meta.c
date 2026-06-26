#include "lt_ota_meta.h"
#include "lt_crc32.h"
#include <string.h>

uint32_t lt_ota_bank_offset(uint8_t bank) {
	return bank == 1 ? LT_OTA_BANK_B_OFF : LT_OTA_BANK_A_OFF;
}

static uint32_t meta_crc(const lt_ota_meta_t *m) {
	// CRC over everything except the trailing crc32 field.
	return lt_crc32(0xFFFFFFFFu, m, offsetof(lt_ota_meta_t, crc32)) ^ 0xFFFFFFFFu;
}

static bool slot_load(const lt_ota_flash_ops_t *ops, uint32_t off, lt_ota_meta_t *out) {
	if (ops->read(off, out, sizeof(*out)) != 0)
		return false;
	if (out->magic != LT_OTA_META_MAGIC)
		return false;
	return out->crc32 == meta_crc(out);
}

bool lt_ota_meta_load(const lt_ota_flash_ops_t *ops, lt_ota_meta_t *out) {
	lt_ota_meta_t a, b;
	bool va = slot_load(ops, LT_OTA_META_SLOT0_OFF, &a);
	bool vb = slot_load(ops, LT_OTA_META_SLOT1_OFF, &b);
	if (va && vb) {
		// higher seq wins; signed diff handles wrap
		*out = ((int32_t)(b.seq - a.seq) > 0) ? b : a;
		return true;
	}
	if (va) {
		*out = a;
		return true;
	}
	if (vb) {
		*out = b;
		return true;
	}
	return false;
}

bool lt_ota_meta_store(const lt_ota_flash_ops_t *ops, lt_ota_meta_t *m) {
	// Determine current newest slot to pick the OTHER one for the new write.
	lt_ota_meta_t cur;
	uint32_t target	  = LT_OTA_META_SLOT0_OFF;
	uint32_t next_seq = 1;
	if (lt_ota_meta_load(ops, &cur)) {
		next_seq = cur.seq + 1;
		// write to whichever slot did NOT supply 'cur' (compare by re-reading)
		lt_ota_meta_t s0;
		bool v0 = slot_load(ops, LT_OTA_META_SLOT0_OFF, &s0);
		target	= (v0 && s0.seq == cur.seq) ? LT_OTA_META_SLOT1_OFF : LT_OTA_META_SLOT0_OFF;
	}
	m->magic = LT_OTA_META_MAGIC;
	m->seq	 = next_seq;
	m->crc32 = meta_crc(m);
	if (ops->erase(target, LT_OTA_META_SLOT_LEN) != 0)
		return false;
	return ops->write(target, m, sizeof(*m)) == 0;
}

bool lt_ota_meta_bank_valid(const lt_ota_flash_ops_t *ops, const lt_ota_meta_t *m, uint8_t bank) {
	if (bank >= LT_OTA_BANK_COUNT)
		return false;
	const lt_ota_bank_desc_t *d = &m->bank[bank];
	if (d->state == LT_OTA_BANK_EMPTY || d->length == 0 || d->length > LT_OTA_BANK_LEN)
		return false;
	uint32_t base = lt_ota_bank_offset(bank);
	uint32_t crc  = 0xFFFFFFFFu;
	uint8_t buf[256];
	uint32_t rem = d->length, off = 0;
	while (rem) {
		uint32_t n = rem < sizeof(buf) ? rem : sizeof(buf);
		if (ops->read(base + off, buf, n) != 0)
			return false;
		crc = lt_crc32(crc, buf, n);
		off += n;
		rem -= n;
	}
	return (crc ^ 0xFFFFFFFFu) == d->crc32;
}

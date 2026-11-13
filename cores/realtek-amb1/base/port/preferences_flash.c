#include "preferences_flash.h"

#include <string.h>

#define VERIFY_CHUNK 128u

int amb1_preferences_flash_bind(amb1_preferences_flash_t *flash,
	const amb1_preferences_flash_ops_t *ops, const amb1_preferences_flash_policy_t *policy) {
	unsigned i;
	uint8_t nonzero = 0;
	if (!flash)
		return -1;
	/* Copy first so rebinding from fields of the same instance is well-defined. */
	if (ops && policy) {
		amb1_preferences_flash_ops_t saved_ops = *ops;
		amb1_preferences_flash_policy_t saved_policy = *policy;
		memset(flash, 0, sizeof(*flash));
		if (!saved_ops.read || !saved_ops.write || !saved_ops.erase ||
		    !saved_ops.lock || !saved_ops.unlock ||
		    !amb1_preferences_layout_reviewed(&saved_policy.review) ||
		    saved_policy.fresh_snapshot_attested != 1 ||
		    saved_policy.shared_controller_lock_attested != 1 ||
		    (saved_policy.existing_flashdb_attested != 0 && saved_policy.existing_flashdb_attested != 1) ||
		    (saved_policy.initial_format_approved != 0 && saved_policy.initial_format_approved != 1))
			return -1;
		for (i = 0; i < AMB1_PREF_SNAPSHOT_HASH_SIZE; ++i) {
			nonzero |= saved_policy.expected_snapshot_hash[i];
			if (saved_policy.expected_snapshot_hash[i] != saved_policy.fresh_snapshot_hash[i])
				return -1;
		}
		if (!nonzero)
			return -1;
		flash->ops = saved_ops;
		flash->policy = saved_policy;
		flash->bound = 1;
		return 0;
	}
	memset(flash, 0, sizeof(*flash));
	return -1;
}

static int allowed(const amb1_preferences_flash_t *flash, long offset, size_t size, int erase) {
	const uint32_t capacity = AMB1_PREF_KVS_END - AMB1_PREF_KVS_START;
	return flash && flash->bound == 1 && offset >= 0 && (unsigned long)offset < capacity &&
	       size && size <= capacity - (unsigned long)offset &&
	       amb1_preferences_range_allowed(&flash->policy.review,
	           AMB1_PREF_KVS_START + (uint32_t)offset, (uint32_t)size, erase);
}

static int writable(const amb1_preferences_flash_t *flash) {
	return flash->policy.existing_flashdb_attested == 1 || flash->policy.initial_format_approved == 1;
}

int amb1_preferences_flash_format_allowed(const amb1_preferences_flash_t *flash) {
	return flash && flash->bound == 1 && flash->policy.initial_format_approved == 1;
}

int amb1_preferences_flash_read(amb1_preferences_flash_t *flash, long offset, void *buffer, size_t size) {
	int result;
	if (!buffer || !allowed(flash, offset, size, 0) || flash->ops.lock(flash->ops.context))
		return -1;
	result = flash->ops.read(flash->ops.context, AMB1_PREF_KVS_START + (uint32_t)offset, buffer, size);
	flash->ops.unlock(flash->ops.context);
	return result ? -1 : (int)size;
}

int amb1_preferences_flash_write(amb1_preferences_flash_t *flash, long offset,
	const void *buffer, size_t size) {
	uint8_t old[VERIFY_CHUNK];
	const uint8_t *data = buffer;
	size_t pos, n, i;
	uint32_t address;
	int result = -1;
	if (!buffer || !allowed(flash, offset, size, 0) || !writable(flash) ||
	    flash->ops.lock(flash->ops.context))
		return -1;
	address = AMB1_PREF_KVS_START + (uint32_t)offset;
	/* Check the whole request before programming any of it. */
	for (pos = 0; pos < size; pos += n) {
		n = size - pos < sizeof(old) ? size - pos : sizeof(old);
		if (flash->ops.read(flash->ops.context, address + (uint32_t)pos, old, n))
			goto done;
		for (i = 0; i < n; ++i)
			if ((old[i] & data[pos + i]) != data[pos + i])
				goto done;
	}
	for (pos = 0; pos < size; pos += n) {
		n = size - pos < sizeof(old) ? size - pos : sizeof(old);
		if (flash->ops.write(flash->ops.context, address + (uint32_t)pos, data + pos, n) ||
		    flash->ops.read(flash->ops.context, address + (uint32_t)pos, old, n) ||
		    memcmp(old, data + pos, n))
			goto done;
	}
	result = (int)size;
done:
	flash->ops.unlock(flash->ops.context);
	return result;
}

int amb1_preferences_flash_erase(amb1_preferences_flash_t *flash, long offset, size_t size) {
	uint8_t data[VERIFY_CHUNK];
	size_t pos, i;
	uint32_t address;
	int result = -1;
	if (!allowed(flash, offset, size, 1) || !writable(flash) || flash->ops.lock(flash->ops.context))
		return -1;
	address = AMB1_PREF_KVS_START + (uint32_t)offset;
	for (pos = 0; pos < size; pos += AMB1_PREF_SECTOR_SIZE)
		if (flash->ops.erase(flash->ops.context, address + (uint32_t)pos, AMB1_PREF_SECTOR_SIZE))
			goto done;
	for (pos = 0; pos < size; pos += sizeof(data)) {
		if (flash->ops.read(flash->ops.context, address + (uint32_t)pos, data, sizeof(data)))
			goto done;
		for (i = 0; i < sizeof(data); ++i)
			if (data[i] != 0xff)
				goto done;
	}
	result = (int)size;
done:
	flash->ops.unlock(flash->ops.context);
	return result;
}

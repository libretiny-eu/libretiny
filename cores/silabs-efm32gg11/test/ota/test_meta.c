#include "lt_ota_meta.h"
#include <assert.h>
#include <string.h>

// Host shim (Step 4): a 2 MB RAM-backed flash with page erase.
extern lt_ota_flash_ops_t host_flash_ops;
extern void host_flash_reset(void); // all-0xFF
extern uint8_t *host_flash_base(void);

int main(void) {
	host_flash_reset();
	lt_ota_meta_t m;

	// Empty flash -> no valid metadata.
	assert(!lt_ota_meta_load(&host_flash_ops, &m));

	// Store one record -> loads back with seq 1.
	memset(&m, 0, sizeof(m));
	m.magic		= LT_OTA_META_MAGIC;
	m.boot_bank = 0;
	m.attempts	= 3;
	assert(lt_ota_meta_store(&host_flash_ops, &m));
	lt_ota_meta_t r;
	assert(lt_ota_meta_load(&host_flash_ops, &r));
	assert(r.boot_bank == 0 && r.seq >= 1);

	// Store again -> goes to the OTHER slot, higher seq wins.
	m			= r;
	m.boot_bank = 1;
	assert(lt_ota_meta_store(&host_flash_ops, &m));
	assert(lt_ota_meta_load(&host_flash_ops, &r));
	assert(r.boot_bank == 1 && r.seq >= 2);

	// Torn write in the newest slot -> loader falls back to the older valid slot.
	// Corrupt slot1 (the one we just wrote) by flipping a byte in its CRC region.
	uint8_t *f = host_flash_base();
	f[LT_OTA_META_SLOT1_OFF + 8] ^= 0xFF; // boot_bank byte; CRC now mismatches
	assert(lt_ota_meta_load(&host_flash_ops, &r));
	assert(r.boot_bank == 0); // fell back to slot0 (seq 1)
	return 0;
}

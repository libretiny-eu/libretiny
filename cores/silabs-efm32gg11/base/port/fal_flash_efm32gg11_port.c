/* Phase 1 stub FAL flash port for silabs-efm32gg11.
 *
 * cores/common/base/config/fal_cfg.h declares `extern const struct fal_flash_dev flash0`
 * and lt_main() calls fal_init() unconditionally — so we must provide the
 * symbol even though Phase 1 has no flash IO surface yet. The ops below return
 * failure for every IO call; the partition table in fal_cfg.h is empty for this
 * family, so fal_init() just records a no-op flash device and lt_main()'s
 * `fal_partition_find("root")` returns NULL (which is the expected Phase 1
 * state — sketches don't touch FAL).
 *
 * Phase 2 will replace these stubs with emlib MSC-backed read/write/erase
 * (internal flash @ 0x00000000, 4 KiB pages, 2 MiB on EFM32GG11B820).
 */

#include "em_device.h"
#include "lt_family.h"

#include <fal.h>
#include <libretiny.h>

#define FLASH_ERASE_MIN_SIZE (4 * 1024)
#ifndef FLASH_LENGTH
#define FLASH_LENGTH (2 * 1024 * 1024) // EFM32GG11B820 internal flash
#endif

static int init(void) {
	return 0;
}

static int read(long offset, uint8_t *buf, size_t size) {
	(void)offset;
	(void)buf;
	(void)size;
	return -1;
}

static int write(long offset, const uint8_t *buf, size_t size) {
	(void)offset;
	(void)buf;
	(void)size;
	return -1;
}

static int erase(long offset, size_t size) {
	(void)offset;
	(void)size;
	return -1;
}

const struct fal_flash_dev flash0 = {
	.name		= FAL_FLASH_DEV_NAME,
	.addr		= 0x0,
	.len		= FLASH_LENGTH,
	.blk_size	= FLASH_ERASE_MIN_SIZE,
	.ops		= {init, read, write, erase},
	.write_gran = 1,
};

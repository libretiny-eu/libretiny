/* Experimental SDK qualification binding; deliberately not builder-linked. */
#ifndef AMB1_IMAGE2_SDK_ADAPTER_H
#define AMB1_IMAGE2_SDK_ADAPTER_H

#include "image2_flash_backend.h"

#ifndef LT_AMB1_EXPERIMENTAL_IMAGE2_SDK_ADAPTER
#define LT_AMB1_EXPERIMENTAL_IMAGE2_SDK_ADAPTER 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define IMAGE2_SDK_SCRATCH_MAX 4u
typedef struct {
	uint32_t read_start, read_size;
	/* Explicitly approved expendable 4096-byte sector offsets, copied at bind.
	 * NULL/zero is read-only. Never include firmware, settings or recovery data.
	 * This permits qualification experiments, NOT production OTA/demotion.
	 */
	const uint32_t *scratch_sectors;
	size_t scratch_count;
} image2_sdk_qualification_policy;

/* Enabled builds require the reviewed image2_controller_lock.patch applied to
 * the selected SDK. Before concurrent users, startup must successfully call
 * device_mutex_prepare(RT_DEV_LOCK_FLASH). No fallback/private mutex exists.
 * Bind once from a dedicated normal task, using an initially empty ops object.
 * No ISR/timer/network callback use. All other flash users must cooperate or be
 * quiesced (including currently unlocked FAL and chip-ID paths). No allocation,
 * logging, network or callback work occurs under the lock in this adapter.
 *
 * Off by default: bind returns 0 and clears ops without referring to SDK symbols.
 * On: callbacks use the shared SDK mutex open-to-close; failures latch until
 * close, which must run in the binding task. Context is a private singleton,
 * never copied/reset; one acquisition per boot, including failed operations.
 * I/O bounds are finite, but opaque synchronous SDK/ROM waits ARE NOT timed out.
 * Existing --wrap denials remain; only private, held scratch capabilities call
 * __real_flash_erase_sector / __real_flash_burst_write. Preserve both wrap flags
 * at link time. No general write facade or stream/word-write bypass is supplied.
 */
int image2_sdk_adapter_bind(const image2_sdk_qualification_policy *, image2_flash_sdkops *);

#ifdef __cplusplus
}
#endif
#endif

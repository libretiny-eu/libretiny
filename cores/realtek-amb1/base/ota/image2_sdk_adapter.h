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
#define IMAGE2_SDK_DENY_MAX	   4u

/* Half-open [start, end) staging window. {0, 0} means "no staging window" and
 * keeps the scratch-only policy; every other value must be sector-aligned. */
typedef struct {
	uint32_t start, end;
} image2_sdk_range;

typedef struct {
	uint32_t read_start, read_size;
	/* Explicitly approved expendable 4096-byte sector offsets, copied at bind.
	 * NULL/zero is read-only. Never include firmware, settings or recovery data.
	 * This permits qualification experiments, NOT production OTA/demotion.
	 */
	const uint32_t *scratch_sectors;
	size_t scratch_count;
	/* Optional inactive-slot staging authorization, copied at bind. Writes in
	 * this window are allowed only when the request is wholly contained AND
	 * overlaps none of deny_ranges. The caller (the family binding) must derive
	 * the window from the observed running slot and must deny that running slot,
	 * the boot region and the KVS region. This adapter does not infer slots.
	 */
	image2_sdk_range staging;
	const image2_sdk_range *deny_ranges;
	size_t deny_count;
	/* Observed running slot, copied at bind. This is a SEPARATE capability from
	 * scratch/staging: it is never writable through program() or erase_sector(),
	 * even if a scratch sector or the staging window happens to overlap it.
	 * {0, 0} means "no running-slot source" (read-only/staging-only policy).
	 */
	image2_sdk_range source;
	/* Optional one-byte activation demote address, inside source. When non-zero
	 * it must equal source.start + 8 (the first byte of the running signature)
	 * and enables demote(): the ONLY mutation ever permitted in source. Zero
	 * disables demote and keeps this a staging-only adapter.
	 */
	uint32_t demote_address;
	/* Required whenever demote_address is set, and zero otherwise. It must equal
	 * staging.start + 8 (the first byte of the inactive-slot signature) with all
	 * eight signature bytes inside the staging window. demote() reads this
	 * address and refuses unless the new signature is present there, so a direct
	 * adapter call cannot demote before the target image is published.
	 * This is verification evidence, not an extra write capability.
	 */
	uint32_t target_signature_address;
} image2_sdk_qualification_policy;

/* Enabled builds require the reviewed image2_controller_lock.patch applied to
 * the selected SDK. Before concurrent users, startup must successfully call
 * device_mutex_prepare(RT_DEV_LOCK_FLASH). No fallback/private mutex exists.
 * Bind once from a dedicated normal task, using an initially empty ops object.
 * No ISR/timer/network callback use. All other flash users must cooperate or be
 * quiesced (including currently unlocked FAL and chip-ID paths). No allocation,
 * logging, network or callback work occurs under the lock in this adapter.
 *
 * Authorization is the union of two disjoint mutation capabilities:
 *   - scratch_sectors: expendable qualification sectors (existing behaviour);
 *   - staging: one caller-supplied half-open window, further restricted by
 *     deny_ranges (the running slot, boot and KVS in the AMB1 binding).
 * source is a third, non-generic capability: it is never writable through
 * program()/erase_sector(), and only the dedicated demote() entry point may
 * change the single demote_address byte (source.start + 8), after a staged
 * write. demote() additionally requires target-published evidence: the
 * inactive-slot signature at target_signature_address (staging.start + 8) must
 * read the new signature, mirroring the backend's publication check. Bind
 * rejects malformed windows, any staging/source overlap, any demote address that
 * is not the exact inside-source signature byte, and any demote that lacks a
 * matching, in-window target-signature address. A request
 * that is not wholly inside a capability, or that touches a deny range, is
 * refused immediately before the guarded SDK call.
 *
 * Off by default: bind returns 0 and clears ops without referring to SDK symbols.
 * On: callbacks use the shared SDK mutex open-to-close; failures latch until
 * close, which must run in the binding task. Context is a private singleton,
 * never copied/reset; one acquisition per boot, including failed operations.
 * I/O bounds are finite, but opaque synchronous SDK/ROM waits ARE NOT timed out.
 * Existing --wrap denials remain; only private, held capabilities call
 * __real_flash_erase_sector / __real_flash_burst_write. Preserve both wrap flags
 * at link time. No general write facade or stream/word-write bypass is supplied.
 */
int image2_sdk_adapter_bind(const image2_sdk_qualification_policy *, image2_flash_sdkops *);

#ifdef __cplusplus
}
#endif
#endif

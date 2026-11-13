/* Strong AMB1 SDK binding for the family OTA flash seam.
 *
 * This file is the reviewed integration that supplies a live sdkops object to
 * lt_ota_family_begin(). It is compiled only for the qualification build
 * (LT_AMB1_EXPERIMENTAL_IMAGE2_SDK_ADAPTER), and it never replaces the weak
 * fail-closed default when that build is inactive.
 *
 * Two independent safeguards keep the running image immutable:
 *   1. bind-time selector evidence: the running slot must read the new
 *      signature and the inactive slot must read the old/erased signature;
 *   2. hand-off re-check: every sdkops() call re-reads the boot source and
 *      returns NULL if it no longer matches the slot observed at bind.
 * The adapter is then authorized to write ONLY the observed inactive slot, and
 * explicitly denies that running slot, the boot region and the KVS region for
 * generic program/erase. The running slot is exposed as a separate source
 * capability whose only permitted mutation is the dedicated one-byte demote at
 * running_start + 8 after activation publishes the target signature.
 *
 * The boot source itself is provided by lt_ota_dual_get_current(). The current
 * AMB1 stub returns 0 unconditionally, so real B-running hardware fails the
 * evidence check and stays fail-closed until a reviewed boot-source probe lands.
 */
#ifndef AMB1_LT_OTA_FAMILY_AMB1_SDK_H
#define AMB1_LT_OTA_FAMILY_AMB1_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Startup (single task, before concurrent flash users): prepare the shared SDK
 * flash mutex. Idempotent, latches its result for the boot and returns exactly
 * 1 when ready. Called from fal_init(); failure keeps the seam fail-closed. */
int lt_ota_family_amb1_sdk_prepare(void);

#ifdef __cplusplus
}
#endif
#endif

/* Experimental injected controller; no SDK binding and not builder-linked. */
#ifndef AMB1_IMAGE2_FLASH_BACKEND_H
#define AMB1_IMAGE2_FLASH_BACKEND_H

#include "image2_transaction.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void *ctx;
	/* Nonblocking global controller lock, held from open through close, including
	 * errors. ALL users (Wi-Fi SDK reads included) must honor it. Return exactly
	 * 1 only after acquisition; failure must acquire nothing. No callback reentry.
	 */
	int (*try_lock)(void *, const void *owner);
	void (*unlock)(void *, const void *owner);
	/* Exact byte operations, offsets not mapped CPU addresses. Return exactly 1
	 * on success. Even errors may have partial effects. Program must touch ONLY
	 * these bytes, within one page; no implicit erase or unreviewed word RMW.
	 * Buffers need not be aligned. Sector erase is the ONLY erase primitive.
	 * A future void-SDK erase adapter needs its own completion/error policy;
	 * this contract does not pretend the SDK provides a return status.
	 */
	int (*read)(void *, uint32_t, void *, size_t);
	int (*program)(void *, uint32_t, const void *, size_t);
	int (*erase_sector)(void *, uint32_t);
	/* Optional dedicated activation demote. When present it must write exactly
	 * the one source-signature byte the adapter was bound to, and must NOT be a
	 * generic write primitive. NULL keeps the legacy mock path, where the
	 * session drives the single byte through program(); production adapters
	 * supply this so generic program/erase can hard-deny the running slot.
	 * The production SDK adapter also refuses to demote unless the inactive
	 * target has published the new signature, independently of this backend. */
	int (*demote)(void *);
} image2_flash_sdkops;

typedef struct {
	image2_layout layout;
	/* Must equal [inactive.start, inactive.start + ceil(image_size/sector)*sector).
	 * No capability for the rest of the slot, block erase or chip erase.
	 */
	image2_range extent;
	uint32_t image_size, page_size;
	/* Explicit review permits ONLY source.start+8, ASCII 8 -> 0. */
	int allow_source_demote;
} image2_flash_config;

/* One controller per physical chip, initialized ONCE with {0}, retained for the
 * whole boot. Never copy/reset/reinitialize it, including after close/failure.
 * Treat all fields, ops/context, and authorization inputs as immutable while
 * held. Call from one owner task; the injected lock is not an API-call mutex.
 * This is a trusted internal capability, not a sandbox against arbitrary C.
 * Internal to image2_session, which privately bridges crypto context and owns
 * phase transitions, close and the boot-long latch. Application clients must
 * not construct transaction callbacks with these internal primitives.
 */
typedef struct image2_flash_backend {
	const struct image2_flash_backend *identity;
	image2_flash_sdkops ops;
	image2_flash_config config;
	int used, held, failed, busy;
	int phase, publication_attempted;
} image2_flash_backend;

int image2_flash_open(image2_flash_backend *, const image2_flash_config *, const image2_flash_sdkops *);
/* Always close after an attempted open: geometry rejection retains the acquired
 * lock and consumes the controller too. Lock-acquisition failure acquires none.
 * Close never performs cleanup mutations or restores partial flash effects.
 */
void image2_flash_close(image2_flash_backend *);
/* Compatible with transaction read/erase/write callbacks via ctx = backend.
 * Reads allow only the exact eight-byte active signature or approved extent.
 * Every failure latches the session; zero lengths are rejected without SDK I/O.
 */
int image2_flash_read(void *, uint32_t, void *, size_t);
int image2_flash_erase(void *, uint32_t, size_t);
int image2_flash_write(void *, uint32_t, const void *, size_t);
/* Internal session capability API, not application entry points. Phase 1 is
 * staging, 2 verification (read only), 3 publication, 4 terminal. */
int image2_flash_phase(image2_flash_backend *, int);
int image2_flash_demote(image2_flash_backend *);

/* Hardware binding intentionally unsupported. SDK rtl8195a flash_api.c uses
 * global flashobj/isinit and controller pin/state changes for reads too; an OTA
 * mutex alone cannot serialize Wi-Fi or ROM users. Production additionally
 * needs reviewed geometry/boot source, global SDK exclusion, completion/cache/
 * execution safety, and a narrowly scoped guard-approved adapter. No __real
 * symbols, guard changes, or direct hardware fallback live in this module.
 */
#ifdef __cplusplus
}
#endif
#endif

/* Copyright (c) Kuba Szczodrzyński 2023-03-09. */

#pragma once

#include <libretiny.h>
#include <uf2ota/uf2types.h>

/**
 * @brief Chip's OTA type enumeration.
 */
typedef enum {
	OTA_TYPE_SINGLE = 0,
	OTA_TYPE_DUAL	= 1,
	OTA_TYPE_FILE	= 2,
} lt_ota_type_t;

/**
 * @brief OTA update process context.
 */
typedef struct {
	uf2_ota_t uf2;
	uf2_info_t info;
	uint8_t buf[UF2_BLOCK_SIZE];   // block data buffer
	uint8_t *buf_pos;			   // buffer writing position
	uint32_t bytes_written;		   // update progress
	uint32_t bytes_total;		   // total update size
	uf2_err_t error;			   // LT OTA/uf2ota error code
	bool running;				   // whether update has begun
	bool family_handled;		   // family lifecycle hook owns this update
	bool family_finished;		   // family end hook completed successfully
	uf2_err_t family_error;		   // sticky family hook error
	void (*callback)(void *param); // progress callback
	void *callback_param;		   // callback argument
} lt_ota_ctx_t;

/**
 * @brief Result of a family OTA lifecycle hook.
 *
 * LT_OTA_FAMILY_PASS (0) means "not handled"; the common path keeps its normal
 * behaviour. LT_OTA_FAMILY_HANDLED means the family accepted/staged the item and
 * the stock FAL writer must not run. Every other value must be a uf2_err_t error
 * code greater than UF2_ERR_IGNORE; out-of-range values are latched as
 * UF2_ERR_WRITE_FAILED so they stay safe for the Update error mapping.
 */
typedef int lt_ota_family_result_t;

#define LT_OTA_FAMILY_PASS	  0
#define LT_OTA_FAMILY_HANDLED 0x1000

/*
 * Family OTA lifecycle hooks (weak, opt-in; additive to the common OTA path).
 *
 * Families that need staged/authenticated activation (for example AMB1 image2)
 * override all four hooks. Other families link unchanged because the weak
 * defaults below are no-ops.
 *
 * Contract guaranteed by the common path:
 *  - begin() is called once, after the UF2 context and write protection are
 *    initialized, before any block is processed.
 *  - write_block() is called only for blocks that would otherwise reach the
 *    stock uf2_write() FAL writer: magic/family checked, the UF2 header already
 *    parsed, and the block is main-flash with len > 0. It is not called for the
 *    header block or for blocks the stock writer would ignore.
 *  - end() is called at most once, only when the upload is byte-complete with no
 *    parser/family error and no partially buffered block, and only after the
 *    caller's digest/authentication check has passed (UpdateClass verifies MD5
 *    before calling lt_ota_end()). It must finalize/verify but MUST NOT switch
 *    images, activate, or reboot.
 *  - abort() is called once for an unfinished transaction (begin error, explicit
 *    abort, incomplete upload, parser/family error, digest mismatch) and never
 *    after a successful end(). It must be idempotent and MUST NOT commit,
 *    activate, or reboot.
 *
 * Parser limitation: uf2ota 5.0.0 exposes no parse-only data-block API. A
 * handled block is delivered before the private uf2_parse_block()/uf2_write()
 * step, so OTA_PART_INFO target selection, BINPATCH application and the stock
 * sequence check have not run. The family MUST therefore validate
 * block_seq/block_count/addr/len/contiguity, enforce its own target-slot
 * allowlist (never trust UF2 partition tags), treat the block as read-only, and
 * independently verify the staged image before any activation. A clean extension
 * would expose a public parse-only uf2 entry point (or split uf2_write into
 * validate + sink stages); see lt_ota.c for the exact gap.
 */

/**
 * @brief Begin a family OTA transaction.
 *
 * @param ctx OTA context
 * @param size update file size passed to lt_ota_begin(); 0 if unknown
 * @return LT_OTA_FAMILY_PASS to use the stock writer; LT_OTA_FAMILY_HANDLED to
 * take ownership of every subsequent main-flash block (the common path will not
 * call uf2_write() or lt_ota_switch()); or a uf2_err_t error to refuse the update
 */
lt_ota_family_result_t lt_ota_family_begin(lt_ota_ctx_t *ctx, size_t size);

/**
 * @brief Accept or stage one main-flash UF2 block.
 *
 * The block has passed uf2_check_block() (magic/family) and the header has been
 * parsed, but data-block tags, partition selection and binpatch have NOT been
 * applied. See the parser limitation above.
 *
 * @param ctx OTA context
 * @param block block to accept; must be treated as read-only; cannot be NULL.
 * The pointer and its data are only valid for the duration of the call.
 * @return LT_OTA_FAMILY_HANDLED if accepted/staged (the stock FAL writer is
 * skipped); a uf2_err_t error to abort the upload. LT_OTA_FAMILY_PASS after
 * begin() returned HANDLED is a contract violation and latches
 * UF2_ERR_WRITE_FAILED.
 */
lt_ota_family_result_t lt_ota_family_write_block(lt_ota_ctx_t *ctx, const uf2_block_t *block);

/**
 * @brief Finalize a family OTA transaction after the caller's digest check.
 *
 * Must verify/finalize the staged image only. It must not activate, switch
 * images, or reboot; activation is a separate, explicit family operation.
 *
 * @param ctx OTA context
 * @return LT_OTA_FAMILY_HANDLED on success; a uf2_err_t error on failure.
 * LT_OTA_FAMILY_PASS after begin() returned HANDLED is treated as a contract
 * violation and latches UF2_ERR_WRITE_FAILED.
 */
lt_ota_family_result_t lt_ota_family_end(lt_ota_ctx_t *ctx);

/**
 * @brief Abandon an unfinished family OTA transaction.
 *
 * Called at most once, and never after a successful lt_ota_family_end(). Must be
 * idempotent and must never commit, activate, or reboot.
 *
 * @param ctx OTA context
 */
void lt_ota_family_abort(lt_ota_ctx_t *ctx);

/**
 * @brief Initialize the update context to begin OTA process.
 *
 * @param ctx OTA context
 * @param size length of the update file; 0 if unknown
 */
void lt_ota_begin(lt_ota_ctx_t *ctx, size_t size);

/**
 * @brief Release the update context without attempting activation. Safe to repeat.
 * This does not undo flash writes or family-specific status set during streaming.
 *
 * @param ctx OTA context
 */
void lt_ota_abort(lt_ota_ctx_t *ctx);

/**
 * @brief Finish the update process. If the update has been written completely,
 * try to activate the target image. Free allocated internal structures, regardless
 * of the activation result. Parser errors or buffered trailing bytes prevent
 * activation. Callers must validate any expected digest before calling this.
 *
 * @param ctx OTA context
 * @return false if activation was attempted and not successful; true otherwise
 * (including incomplete/rejected updates, for compatibility)
 */
bool lt_ota_end(lt_ota_ctx_t *ctx);

/**
 * @brief Set family-specific, write-protected flash areas in the OTA update context.
 * This shouldn't be called manually, as it's done by lt_ota_begin().
 *
 * @param uf2 uf2ota context
 */
void lt_ota_set_write_protect(uf2_ota_t *uf2);

/**
 * @brief Process a chunk of data.
 *
 * Data is written to the buffer, unless a full UF2 block is already available,
 * in which case it's also processed by UF2OTA and written to flash.
 *
 * It's advised to write in 512-byte chunks (or its multiples).
 *
 * @param ctx OTA context
 * @param data chunk of bytes to process
 * @param len size of the chunk
 * @return number of bytes correctly processed; should equal 'len' in case of no errors
 */
size_t lt_ota_write(lt_ota_ctx_t *ctx, const uint8_t *data, size_t len);

/**
 * @brief Try to write the block. In case of UF2 errors, error code is set in the context.
 * Note: use lt_ota_write() instead. This is for internal usage only.
 *
 * @param block UF2 block to check and write; cannot be NULL
 * @return whether no error has occurred
 */
bool lt_ota_write_block(lt_ota_ctx_t *ctx, uf2_block_t *block);

/**
 * @brief Get OTA type of the device's chip.
 */
lt_ota_type_t lt_ota_get_type();

/**
 * @brief Check if the specified OTA image is valid.
 *
 * @param index OTA index to check; 0 for single-OTA chips, 1 or 2 for dual-OTA chips
 * @return true if index is valid for the chip's OTA type, and there is a valid image; false otherwise
 */
bool lt_ota_is_valid(uint8_t index);

/**
 * @brief Check if OTA rollback is possible (switching the stored index to another partition).
 *
 * Note that this is not the same as "switching" OTA with revert=true.
 *
 * @return true if 2nd image is valid and the chip is dual-OTA; false otherwise
 */
bool lt_ota_can_rollback();

/**
 * @brief Get the currently running firmware's OTA index.
 *
 * @return OTA index if dual-OTA is supported, 0 otherwise
 */
uint8_t lt_ota_dual_get_current();

/**
 * @brief Read the currently active OTA index, i.e. the one that will boot upon restart.
 *
 * @return OTA index if dual-OTA is supported, 0 otherwise
 */
uint8_t lt_ota_dual_get_stored();

/**
 * @brief Check which UF2 OTA scheme should be used for applying firmware updates.
 *
 * @return OTA scheme of the target partition
 */
uf2_ota_scheme_t lt_ota_get_uf2_scheme();

/**
 * @brief Try to switch OTA index to the other image. For single-OTA chips, only check if the upgrade image is valid.
 *
 * This can be used to "activate" the upgrade after flashing.
 *
 * @param revert switch if (and only if) the other image is already marked as active (i.e.
 * switch back to the running image)
 * @return false if the second image (or upgrade image) is not valid; false if writing failed; true otherwise
 */
bool lt_ota_switch(bool revert);

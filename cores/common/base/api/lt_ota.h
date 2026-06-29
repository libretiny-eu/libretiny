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
	void (*callback)(void *param); // progress callback
	void *callback_param;		   // callback argument
} lt_ota_ctx_t;

/**
 * @brief Initialize the update context to begin OTA process.
 *
 * @param ctx OTA context
 * @param size length of the update file; 0 if unknown
 */
void lt_ota_begin(lt_ota_ctx_t *ctx, size_t size);

/**
 * @brief Finish the update process. If the update has been written completely,
 * try to activate the target image. Free allocated internal structures, regardless
 * of the activation result.
 *
 * @param ctx OTA context
 * @return false if activation was attempted and not successful; true otherwise
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

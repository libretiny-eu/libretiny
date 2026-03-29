/* Copyright (c) Kuba Szczodrzyński 2022-05-30. */

#include "Update.h"

#include <utility>

extern "C" {
#include <fal.h>
}

static const char *errUf2Text[] = {
	nullptr,					 // UF2_ERR_OK (0)
	nullptr,					 // UF2_ERR_IGNORE (1)
	"Bad Magic Number",			 // UF2_ERR_MAGIC (2)
	"Bad Family ID",			 // UF2_ERR_FAMILY (3)
	"Missing Header",			 // UF2_ERR_NOT_HEADER (4)
	"Old OTA Format Found",		 // UF2_ERR_OTA_VER (5)
	"Image Not Applicable",		 // UF2_ERR_OTA_WRONG (6)
	"Partition Not Found",		 // UF2_ERR_PART_404 (7)
	"Partition Info Invalid",	 // UF2_ERR_PART_INVALID (8)
	"Partition Info Missing",	 // UF2_ERR_PART_UNSET (9)
	"Block Data Too Long",		 // UF2_ERR_DATA_TOO_LONG (10)
	"Bad Block Sequence Number", // UF2_ERR_SEQ_MISMATCH (11)
	"Flash Erase Failed",		 // UF2_ERR_ERASE_FAILED (12)
	"Flash Write Failed",		 // UF2_ERR_WRITE_FAILED (13)
	"Write Failed Length",		 // UF2_ERR_WRITE_LENGTH (14)
	"Partition Write-Protected", // UF2_ERR_WRITE_PROTECT (15)
	"Memory Alloc Failed",		 // UF2_ERR_ALLOC_FAILED (16)
};

static const char *errArdText[] = {
	nullptr,						   // UPDATE_ERROR_OK (0)
	nullptr,						   // UPDATE_ERROR_WRITE (1)
	nullptr,						   // UPDATE_ERROR_ERASE (2)
	nullptr,						   // UPDATE_ERROR_READ (3)
	nullptr,						   // UPDATE_ERROR_SPACE (4)
	"Bad Size Given",				   // UPDATE_ERROR_SIZE (5)
	"Stream Read Timeout",			   // UPDATE_ERROR_STREAM (6)
	"MD5 Check Failed",				   // UPDATE_ERROR_MD5 (7)
	nullptr,						   // UPDATE_ERROR_MAGIC_BYTE (8)
	"Could Not Activate The Firmware", // UPDATE_ERROR_ACTIVATE (9)
	nullptr,						   // UPDATE_ERROR_NO_PARTITION (10)
	"Bad Argument",					   // UPDATE_ERROR_BAD_ARGUMENT (11)
	"Aborted",						   // UPDATE_ERROR_ABORT (12)
};

static char errorStr[14];

/**
 * @brief Set the callback invoked after writing data to flash.
 */
UpdateClass &UpdateClass::onProgress(THandlerFunction_Progress handler) {
	this->callback = std::move(handler);
	return *this;
}

void UpdateClass::progressHandler(UpdateClass *self) {
	if (self->callback)
		self->callback(self->ctx->bytes_written, self->ctx->bytes_total);
}

/** 
 * @brief Check if OTA rollback is possible (switching the stored index to another partition).
 *
 * Note that this is not the same as "switching" OTA with revert=true.
 *
 * @return true if 2nd image is valid and the chip is dual-OTA; false otherwise
  */
bool UpdateClass::canRollBack() {
	return lt_ota_can_rollback();
}

/** 
 * @brief Try to switch OTA index to the other image. For single-OTA chips, only check if the upgrade image is valid.
 *
 * This can be used to "activate" the upgrade after flashing.
 *
 * @param revert switch if (and only if) the other image is already marked as active (i.e.
 * switch back to the running image)
 * @return false if the second image (or upgrade image) is not valid; false if writing failed; true otherwise
  */
bool UpdateClass::rollBack() {
	if (!lt_ota_can_rollback())
		return false;
	return lt_ota_switch(/* revert= */ false);
}

/**
 * @brief Set the expected MD5 of the firmware (hexadecimal string).
 */
bool UpdateClass::setMD5(const char *md5) {
	if (strlen(md5) != 32)
		return false;
	if (!this->md5Expected)
		this->md5Expected = static_cast<uint8_t *>(malloc(16));
	if (!this->md5Expected)
		return false;
	lt_xtob(md5, 32, this->md5Expected);
	return true;
}

/**
 * @brief Return a hexadecimal string of calculated firmware MD5 sum.
 */
String UpdateClass::md5String() {
	if (!this->md5Digest)
		return "";
	char out[32 + 1];
	lt_btox(this->md5Digest, 16, out);
	return String(out);
}

/**
 * @brief Get calculated MD5 digest of the firmware.
 */
void UpdateClass::md5(uint8_t *result) {
	if (!this->md5Digest) {
		memset(result, '\0', 16);
		return;
	}
	memcpy(result, this->md5Digest, 16);
}

/**
 * @brief Get combined error code of the update.
 */
uint16_t UpdateClass::getErrorCode() const {
	return (this->getError() << 8) | this->getUF2Error();
}

/**
 * @brief Check if any error has occurred (incl. aborting the update).
 */
bool UpdateClass::hasError() const {
	return this->getError() != UPDATE_ERROR_OK || this->getUF2Error() > UF2_ERR_IGNORE;
}

/**
 * @brief Clear all errors. This is NOT recommended.
 */
void UpdateClass::clearError() {
	this->errArd = UPDATE_ERROR_OK;
	this->errUf2 = UF2_ERR_OK;
	if (this->ctx)
		this->ctx->error = UF2_ERR_OK;
}

/**
 * @brief Get a textual description of the error.
 */
const char *UpdateClass::errorString() const {
	uint8_t err;
	if ((err = this->getUF2Error()) > UF2_ERR_IGNORE)
		return errUf2Text[err];
	if ((err = this->getError()) != UPDATE_ERROR_OK)
		return errArdText[err];
	if (!this->hasError())
		return "";
	sprintf(errorStr, "ard=%u,uf2=%u", this->getError(), this->getUF2Error());
	return errorStr;
}

/**
 * @brief Print string error info to the stream.
 */
void UpdateClass::printError(Print &out) const {
	out.println(errorString());
}

/**
 * @brief Print details about the error and current OTA state.
 */
void UpdateClass::printErrorContext() {
#if LT_DEBUG_OTA
	if (!this->ctx)
		return;

	LT_EM(OTA, "Error: %s", errorString());
	if (errArd == UPDATE_ERROR_ABORT)
		return;

	LT_EM(OTA, "- written: %u of %u", this->ctx->bytes_written, this->ctx->bytes_total);
	LT_EM(
		OTA,
		"- buf: size=%lld, left=%lld",
		this->ctx->buf_pos - this->ctx->buf,
		this->ctx->buf + UF2_BLOCK_SIZE - this->ctx->buf_pos
	);
	hexdump(this->ctx->buf, this->ctx->buf_pos - this->ctx->buf);

	if (ctx)
		LT_EM(
			OTA,
			"- ctx: seq=%u, part=%s",
			this->ctx->uf2.seq - 1, // print last parsed block seq
			this->ctx->uf2.part ? this->ctx->uf2.part->name : nullptr
		);

	auto *block = (uf2_block_t *)this->ctx->buf;
	LT_EM(OTA, "- buf: seq=%u/%u, addr=%u, len=%u", block->block_seq, block->block_count, block->addr, block->len);
#endif
}

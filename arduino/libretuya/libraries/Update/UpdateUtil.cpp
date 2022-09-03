/* Copyright (c) Kuba Szczodrzyński 2022-05-30. */

#include "Update.h"

static const uint8_t errorMap[] = {
	UPDATE_ERROR_OK,		   /* UF2_ERR_OK - no error */
	UPDATE_ERROR_OK,		   /* UF2_ERR_IGNORE - block should be ignored */
	UPDATE_ERROR_MAGIC_BYTE,   /* UF2_ERR_MAGIC - wrong magic numbers */
	UPDATE_ERROR_BAD_ARGUMENT, /* UF2_ERR_FAMILY - family ID mismatched */
	UPDATE_ERROR_BAD_ARGUMENT, /* UF2_ERR_NOT_HEADER - block is not a header */
	UPDATE_ERROR_BAD_ARGUMENT, /* UF2_ERR_OTA_VER - unknown/invalid OTA format version */
	UPDATE_ERROR_MAGIC_BYTE,   /* UF2_ERR_OTA_WRONG - no data for current OTA index */
	UPDATE_ERROR_NO_PARTITION, /* UF2_ERR_PART_404 - no partition with that name */
	UPDATE_ERROR_BAD_ARGUMENT, /* UF2_ERR_PART_ONE - only one partition tag in a block */
	UPDATE_ERROR_BAD_ARGUMENT, /* UF2_ERR_PART_UNSET - attempted to write without target partition */
	UPDATE_ERROR_BAD_ARGUMENT, /* UF2_ERR_DATA_TOO_LONG - data too long - tags won't fit */
	UPDATE_ERROR_BAD_ARGUMENT, /* UF2_ERR_SEQ_MISMATCH - sequence number mismatched */
	UPDATE_ERROR_ERASE,		   /* UF2_ERR_ERASE_FAILED - erasing flash failed */
	UPDATE_ERROR_WRITE,		   /* UF2_ERR_WRITE_FAILED - writing to flash failed */
	UPDATE_ERROR_WRITE		   /* UF2_ERR_WRITE_LENGTH - wrote fewer data than requested */
};

static char errorStr[14];

/**
 * @brief Set the callback invoked after writing data to flash.
 */
UpdateClass &UpdateClass::onProgress(THandlerFunction_Progress callback) {
	this->callback = callback;
	return *this;
}

void UpdateClass::cleanup(uint8_t ardErr, uf2_err_t uf2Err) {
	errUf2 = uf2Err;
	errArd = ardErr;

#if LT_DEBUG_OTA
	if (hasError())
		printErrorContext1();
#endif

	free(ctx); // NULL in constructor
	ctx = NULL;
	uf2_info_free(info); // NULL in constructor
	info = NULL;
	free(buf); // NULL in constructor
	buf = bufPos = NULL;

	bytesWritten = 0;
	bytesTotal	 = 0;
}

/**
 * @brief Check for UF2 errors. Set errArd and errUf2 in case of errors.
 * Ignored blocks are not reported as errors.
 * Abort the update.
 * Use like: "if (errorUf2(...)) return false;"
 * @return true if err is not OK, false otherwise
 */
bool UpdateClass::checkUf2Error(uf2_err_t err) {
	if (err <= UF2_ERR_IGNORE)
		return false;
	cleanup(errorMap[err], err);
	return true;
}

/**
 * @brief Abort the update with UPDATE_ERROR_ABORT reason.
 */
void UpdateClass::abort() {
	LT_DM(OTA, "Aborting update");
	cleanup(UPDATE_ERROR_ABORT);
}

void UpdateClass::bufAlloc() {
	if (!buf)
		buf = bufPos = (uint8_t *)malloc(UF2_BLOCK_SIZE);
}

uint16_t UpdateClass::bufLeft() {
	return buf + UF2_BLOCK_SIZE - bufPos;
}

uint16_t UpdateClass::bufSize() {
	return bufPos - buf;
}

/**
 * @brief Print string error info to the stream.
 */
void UpdateClass::printError(Print &out) {
	out.println(errorString());
}

/**
 * @brief Print details about the error and current OTA state.
 */
void UpdateClass::printErrorContext1() {
#if LT_DEBUG_OTA
	LT_EM(OTA, "Error: %s", errorString());
	if (errArd == UPDATE_ERROR_ABORT)
		return;

	LT_EM(OTA, "- written: %u of %u", bytesWritten, bytesTotal);
	LT_EM(OTA, "- buf: size=%u, left=%u", bufSize(), bufLeft());
	hexdump(buf, bufSize());

	if (ctx)
		LT_EM(
			OTA,
			"- ctx: seq=%u, part1=%s, part2=%s",
			ctx->seq - 1, // print last parsed block seq
			ctx->part1 ? ctx->part1->name : NULL,
			ctx->part2 ? ctx->part2->name : NULL
		);

	uf2_block_t *block = (uf2_block_t *)buf;
	if (buf)
		LT_EM(OTA, "- buf: seq=%u/%u, addr=%u, len=%u", block->block_seq, block->block_count, block->addr, block->len);
#endif
}

void UpdateClass::printErrorContext2(const uint8_t *data, size_t len) {
#if LT_DEBUG_OTA
	LT_EM(OTA, "- while writing %u bytes", len);
	if (data)
		hexdump(data, len);
#endif
}

/**
 * @brief Get string representation of the error in format
 * "ard=..,uf2=..". Returns "" if no error.
 */
const char *UpdateClass::errorString() {
	if (!errArd && !errUf2)
		return "";
	sprintf(errorStr, "ard=%u,uf2=%u", errArd, errUf2);
	return errorStr;
}

/**
 * @brief Get firmware name from UF2 info.
 */
const char *UpdateClass::getFirmwareName() {
	if (info)
		return info->fw_name;
	return NULL;
}

/**
 * @brief Get firmware version from UF2 info.
 */
const char *UpdateClass::getFirmwareVersion() {
	if (info)
		return info->fw_version;
	return NULL;
}

/**
 * @brief Get LibreTuya version from UF2 info.
 */
const char *UpdateClass::getLibreTuyaVersion() {
	if (info)
		return info->lt_version;
	return NULL;
}

/**
 * @brief Get target board name from UF2 info.
 */
const char *UpdateClass::getBoardName() {
	if (info)
		return info->board;
	return NULL;
}

/**
 * @brief See LT.otaCanRollback() for more info.
 */
bool UpdateClass::canRollBack() {
	return LT.otaCanRollback();
}

/**
 * @brief See LT.otaRollback() for more info.
 */
bool UpdateClass::rollBack() {
	return LT.otaRollback();
}

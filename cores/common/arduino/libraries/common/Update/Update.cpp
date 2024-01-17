/* Copyright (c) Kuba Szczodrzyński 2022-05-29. */

#include "Update.h"

static const UpdateError errorMap[] = {
	UPDATE_ERROR_OK,		   /* UF2_ERR_OK - no error */
	UPDATE_ERROR_OK,		   /* UF2_ERR_IGNORE - block should be ignored */
	UPDATE_ERROR_MAGIC_BYTE,   /* UF2_ERR_MAGIC - wrong magic numbers */
	UPDATE_ERROR_BAD_ARGUMENT, /* UF2_ERR_FAMILY - family ID mismatched */
	UPDATE_ERROR_BAD_ARGUMENT, /* UF2_ERR_NOT_HEADER - block is not a header */
	UPDATE_ERROR_BAD_ARGUMENT, /* UF2_ERR_OTA_VER - unknown/invalid OTA format version */
	UPDATE_ERROR_MAGIC_BYTE,   /* UF2_ERR_OTA_WRONG - no data for current OTA scheme */
	UPDATE_ERROR_NO_PARTITION, /* UF2_ERR_PART_404 - no partition with that name */
	UPDATE_ERROR_BAD_ARGUMENT, /* UF2_ERR_PART_INVALID - invalid partition info tag */
	UPDATE_ERROR_BAD_ARGUMENT, /* UF2_ERR_PART_UNSET - attempted to write without target partition */
	UPDATE_ERROR_BAD_ARGUMENT, /* UF2_ERR_DATA_TOO_LONG - data too long - tags won't fit */
	UPDATE_ERROR_BAD_ARGUMENT, /* UF2_ERR_SEQ_MISMATCH - sequence number mismatched */
	UPDATE_ERROR_ERASE,		   /* UF2_ERR_ERASE_FAILED - erasing flash failed */
	UPDATE_ERROR_WRITE,		   /* UF2_ERR_WRITE_FAILED - writing to flash failed */
	UPDATE_ERROR_WRITE,		   /* UF2_ERR_WRITE_LENGTH - wrote fewer data than requested */
	UPDATE_ERROR_WRITE,		   /* UF2_ERR_WRITE_PROTECT - target area is write-protected */
	UPDATE_ERROR_WRITE,		   /* UF2_ERR_ALLOC_FAILED - dynamic memory allocation failed */
};

/**
 * @brief Initialize the update process.
 *
 * @param size total UF2 file size
 * @param command must be U_FLASH
 * @return false if parameters are invalid or update is running, true otherwise
 */
bool UpdateClass::begin(
	size_t size,
	int command,
	__attribute__((unused)) int ledPin,
	__attribute__((unused)) uint8_t ledOn,
	__attribute__((unused)) const char *label
) {
#if !LT_HAS_OTA
	LT_E("OTA is not yet supported on this chip!");
	this->errArd = UPDATE_ERROR_BAD_ARGUMENT;
	return false;
#endif
	if (this->ctx) {
		return false;
	}
	this->clearError();
	if (size == 0) {
		this->errArd = UPDATE_ERROR_SIZE;
		return false;
	}
	if (command != U_FLASH) {
		this->errArd = UPDATE_ERROR_BAD_ARGUMENT;
		return false;
	}
	if (size == UPDATE_SIZE_UNKNOWN) {
		size = 0;
	}

	this->ctx = static_cast<lt_ota_ctx_t *>(malloc(sizeof(lt_ota_ctx_t)));
	lt_ota_begin(this->ctx, size);
	this->ctx->callback		  = reinterpret_cast<void (*)(void *)>(progressHandler);
	this->ctx->callback_param = this;

	this->md5Ctx = static_cast<LT_MD5_CTX_T *>(malloc(sizeof(LT_MD5_CTX_T)));
	MD5Init(this->md5Ctx);

	return true;
}

/**
 * @brief Finalize the update process. Check for errors and update completion, then activate the new firmware image.
 *
 * @param evenIfRemaining don't raise errors if still in progress
 * @return false in case of errors or no update running; true otherwise
 */
bool UpdateClass::end(bool evenIfRemaining) {
	if (!this->ctx)
		return false;

	// update is running or finished; cleanup and end it
	if (!isFinished() && !evenIfRemaining)
		// abort if not finished
		this->errArd = UPDATE_ERROR_ABORT;

	if (!this->md5Digest)
		this->md5Digest = static_cast<uint8_t *>(malloc(16));
	MD5Final(this->md5Digest, this->md5Ctx);

	this->cleanup(/* clearError= */ evenIfRemaining);
	return !this->hasError();
}

/**
 * @brief Cleanup (free) the update context.
 * Try activating the firmware if possible, set local error codes.
 *
 * @param clearError assume successful finish after correct activation
 */
void UpdateClass::cleanup(bool clearError) {
	if (!this->ctx)
		return;

	if (!lt_ota_end(this->ctx)) {
		// activating firmware failed
		this->errArd = UPDATE_ERROR_ACTIVATE;
		this->errUf2 = UF2_ERR_OK;
	} else if (this->md5Digest && this->md5Expected && memcmp(this->md5Digest, this->md5Expected, 16) != 0) {
		// MD5 doesn't match
		this->errArd = UPDATE_ERROR_MD5;
		this->errUf2 = UF2_ERR_OK;
	} else if (clearError) {
		// successful finish and activation, clear error codes
		this->clearError();
	} else if (this->ctx->error > UF2_ERR_IGNORE) {
		// make error code based on UF2OTA code
		this->errArd = errorMap[this->ctx->error];
		this->errUf2 = this->ctx->error;
	} else {
		// only keep Arduino error code (set by the caller)
		this->errUf2 = UF2_ERR_OK;
	}

#if LT_DEBUG_OTA
	if (this->hasError())
		this->printErrorContext();
#endif

	free(this->ctx);
	this->ctx = nullptr;
	free(this->md5Ctx);
	this->md5Ctx = nullptr;
	free(this->md5Digest);
	this->md5Digest = nullptr;
	free(this->md5Expected);
	this->md5Expected = nullptr;
}

/**
 * @brief Write a chunk of data to the buffer or flash memory.
 *
 * It's advised to write in 512-byte chunks (or its multiples).
 *
 * @param data chunk of data
 * @param len length of the chunk
 * @return size_t amount of bytes written
 */
size_t UpdateClass::write(const uint8_t *data, size_t len) {
	if (!this->ctx)
		return 0;

	MD5Update(this->md5Ctx, data, len);
	size_t written = lt_ota_write(ctx, data, len);
	if (written != len)
		this->cleanup(/* clearError= */ false);
	return written;
}

/**
 * @brief Write all data remaining in the given stream.
 *
 * If the stream doesn't produce any data within UPDATE_TIMEOUT_MS,
 * the update process will be aborted.
 *
 * @param data stream to read from
 * @return size_t amount of bytes written
 */
size_t UpdateClass::writeStream(Stream &data) {
	if (!this->ctx)
		return 0;

	size_t written	  = 0;
	uint32_t lastData = millis();
	// loop until the update is complete
	while (remaining()) {
		// check stream availability
		auto available = data.available();
		if (available <= 0) {
			if (millis() - lastData > UPDATE_TIMEOUT_MS) {
				// waited for data too long; abort with error
				this->errArd = UPDATE_ERROR_STREAM;
				this->cleanup(/* clearError= */ false);
				return written;
			}
			continue;
		}
		// available > 0
		lastData = millis();

		// read data to fit in the remaining buffer space
		auto bufSize = this->ctx->buf_pos - this->ctx->buf;
		auto read	 = data.readBytes(this->ctx->buf_pos, UF2_BLOCK_SIZE - bufSize);
		// update MD5
		MD5Update(this->md5Ctx, this->ctx->buf_pos, read);
		// increment buffer writing head
		this->ctx->buf_pos += read;
		// process the block if complete
		if (bufSize + read == UF2_BLOCK_SIZE)
			lt_ota_write_block(this->ctx, reinterpret_cast<uf2_block_t *>(this->ctx->buf));
		// abort on errors
		if (hasError()) {
			this->cleanup(/* clearError= */ false);
			return written;
		}
		written += read;
	}
	return written;
}

UpdateClass Update;

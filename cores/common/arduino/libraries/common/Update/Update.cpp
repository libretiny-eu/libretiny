/* Copyright (c) Kuba Szczodrzyński 2022-05-29. */

#include "Update.h"

UpdateClass::UpdateClass() : ctx(NULL), info(NULL), buf(NULL) {
	cleanup();
}

/**
 * @brief Initialize the update process.
 *
 * @param size total UF2 file size
 * @param command must be U_FLASH
 * @return false if parameters are invalid or update is running, true otherwise
 */
bool UpdateClass::begin(size_t size, int command, int unused2, uint8_t unused3, const char *unused4) {
	if (ctx)
		return false;
	cleanup();

	LT_DM(OTA, "begin(%u, ...) / OTA curr: %u, trgt: %u", size, LT.otaGetRunning(), LT.otaGetTarget());

	ctx	 = uf2_ctx_init(LT.otaGetTarget(), FAMILY);
	info = uf2_info_init();

	if (!size) {
		cleanup(UPDATE_ERROR_SIZE);
		return false;
	}

	if (command != U_FLASH) {
		cleanup(UPDATE_ERROR_BAD_ARGUMENT);
		return false;
	}

	bytesTotal = size;
	return true;
}

/**
 * @brief Finalize the update process. Check for errors and update completion, then activate the new firmware image.
 *
 * @param evenIfRemaining no idea
 * @return false in case of errors or no update running, true otherwise
 */
bool UpdateClass::end(bool evenIfRemaining) {
	if (hasError() || !ctx)
		// false if not running
		return false;

	if (!isFinished() && !evenIfRemaining) {
		// abort if not finished
		cleanup(UPDATE_ERROR_ABORT);
		return false;
	}
	// TODO what is evenIfRemaining for?
	if (!LT.otaSwitch()) {
		// try to activate the second OTA
		cleanup(UPDATE_ERROR_ACTIVATE);
		return false;
	}

	cleanup();
	return true;
}

/**
 * @brief Write a chunk of data to the buffer or flash memory.
 *
 * It's advised to write in 512-byte chunks (or its multiples).
 *
 * @param data
 * @param len
 * @return size_t
 */
size_t UpdateClass::write(uint8_t *data, size_t len) {
	size_t written = 0;
	if (hasError() || !ctx)
		// 0 if not running
		return 0;

	LT_VM(OTA, "write(%u) / buf %u/512", len, bufSize());

	/* while (buf == bufPos && len >= UF2_BLOCK_SIZE) {
		// buffer empty and entire block is in data
		if (!tryWriteData(data, UF2_BLOCK_SIZE)) {
			// returns 0 if data contains an invalid block
			return written;
		}
		data += UF2_BLOCK_SIZE;
		len -= UF2_BLOCK_SIZE;
		written += UF2_BLOCK_SIZE;
	} */

	// write until buffer space is available
	uint16_t toWrite; // 1..512
	while (len && (toWrite = min(len, bufLeft()))) {
		tryWriteData(data, toWrite);
		if (hasError()) {
			// return on errors
			printErrorContext2(data, toWrite);
			return written;
		}
		data += toWrite;
		len -= toWrite;
		written += toWrite;
	}
	return written;
}

size_t UpdateClass::writeStream(Stream &data) {
	size_t written = 0;
	if (hasError() || !ctx)
		// 0 if not running
		return 0;

	uint32_t lastData = millis();
	// loop until the update is complete
	while (remaining()) {
		// check stream availability
		int available = data.available();
		if (available <= 0) {
			if (millis() - lastData > UPDATE_TIMEOUT_MS) {
				// waited for data too long; abort with error
				cleanup(UPDATE_ERROR_STREAM);
				return written;
			}
			continue;
		}
		// available > 0
		lastData = millis();

		// read data to fit in the remaining buffer space
		bufAlloc();
		uint16_t read = data.readBytes(bufPos, bufLeft());
		bufPos += read;
		written += read;
		tryWriteData();
		if (hasError()) {
			// return on errors
			printErrorContext2(NULL, read); // buf is not valid anymore
			return written;
		}
	}
	return written;
}

/**
 * @brief Try to use the buffer as a block to write. In case of UF2 errors,
 * error codes are set, the update is aborted and 0 is returned
 *
 * @param data received data to copy to buffer or NULL if already in buffer
 * @param len received data length - must be at most bufLeft()
 * @return size_t "used" data size - 0 or 512
 */
size_t UpdateClass::tryWriteData(uint8_t *data, size_t len) {
	uf2_block_t *block = NULL;

	LT_VM(OTA, "Writing %u to buffer (%u/512)", len, bufSize());

	if (len == UF2_BLOCK_SIZE) {
		// data has a complete block
		block = (uf2_block_t *)data;
	} else if (data && len) {
		// data has a part of a block, copy it to buffer
		bufAlloc();
		memcpy(bufPos, data, len);
		bufPos += len;
	}

	if (!block && bufSize() == UF2_BLOCK_SIZE) {
		// use buffer as block (only if not found above)
		block = (uf2_block_t *)buf;
	}

	// a complete block has been found
	if (block) {
		if (checkUf2Error(uf2_check_block(ctx, block)))
			// block is invalid
			return 0;

		if (errUf2 == UF2_ERR_IGNORE)
			// treat ignored blocks as valid
			return UF2_BLOCK_SIZE;

		if (!bytesWritten) {
			// parse header block to allow retrieving firmware info
			if (checkUf2Error(uf2_parse_header(ctx, block, info)))
				// header is invalid
				return 0;

			LT_IM(OTA, "%s v%s - LT v%s @ %s", info->fw_name, info->fw_version, info->lt_version, info->board);

			if (bytesTotal == UPDATE_SIZE_UNKNOWN) {
				// set total update size from block count info
				bytesTotal = block->block_count * UF2_BLOCK_SIZE;
			} else if (bytesTotal != block->block_count * UF2_BLOCK_SIZE) {
				// given update size does not match the block count
				LT_EM(OTA, "Image size wrong; got %u, calculated %u", bytesTotal, block->block_count * UF2_BLOCK_SIZE);
				cleanup(UPDATE_ERROR_SIZE);
				return 0;
			}
		} else {
			// write data blocks normally
			if (checkUf2Error(uf2_write(ctx, block)))
				// block writing failed
				return 0;
		}

		// increment total writing progress
		bytesWritten += UF2_BLOCK_SIZE;
		// call progress callback
		if (callback)
			callback(bytesWritten, bytesTotal);
		// reset the buffer as it's used already
		if (bufSize() == UF2_BLOCK_SIZE)
			bufPos = buf;
		return UF2_BLOCK_SIZE;
	}

	return 0;
}

UpdateClass Update;

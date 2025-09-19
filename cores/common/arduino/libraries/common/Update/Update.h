#pragma once

#include <Arduino.h>
#include <MD5.h>
#include <functional>
#include <uf2ota/uf2ota.h>

typedef enum {
	UPDATE_ERROR_OK			  = 0,	//!< No Error
	UPDATE_ERROR_WRITE		  = 1,	//!< Flash Write Failed
	UPDATE_ERROR_ERASE		  = 2,	//!< Flash Erase Failed
	UPDATE_ERROR_READ		  = 3,	//!< Flash Read Failed
	UPDATE_ERROR_SPACE		  = 4,	//!< Not Enough Space
	UPDATE_ERROR_SIZE		  = 5,	//!< Bad Size Given
	UPDATE_ERROR_STREAM		  = 6,	//!< Stream Read Timeout
	UPDATE_ERROR_MD5		  = 7,	//!< MD5 Check Failed
	UPDATE_ERROR_MAGIC_BYTE	  = 8,	//!< Wrong Magic Byte
	UPDATE_ERROR_ACTIVATE	  = 9,	//!< Could Not Activate The Firmware
	UPDATE_ERROR_NO_PARTITION = 10, //!< Partition Could Not be Found
	UPDATE_ERROR_BAD_ARGUMENT = 11, //!< Bad Argument
	UPDATE_ERROR_ABORT		  = 12, //!< Aborted
} UpdateError;

typedef enum {
	U_FLASH	 = 0,
	U_SPIFFS = 100,
	U_AUTH	 = 200,
} UpdateCommand;

#define UPDATE_SIZE_UNKNOWN 0xFFFFFFFF

#define ENCRYPTED_BLOCK_SIZE 16

#define UPDATE_TIMEOUT_MS 30 * 1000

class UpdateClass {
  public:
	typedef std::function<void(size_t, size_t)> THandlerFunction_Progress;

  public: /* Update.cpp */
	bool begin(
		size_t size		  = UPDATE_SIZE_UNKNOWN,
		int command		  = U_FLASH,
		int ledPin		  = -1,
		uint8_t ledOn	  = LOW,
		const char *label = nullptr
	);
	bool end(bool evenIfRemaining = false);

	size_t write(const uint8_t *data, size_t len);
	size_t writeStream(Stream &data);

  private: /* Update.cpp */
	void cleanup(bool clearError = false);

  public: /* UpdateUtil.cpp */
	UpdateClass &onProgress(THandlerFunction_Progress handler);
	static bool canRollBack();
	static bool rollBack();
	bool setMD5(const char *md5);
	String md5String();
	void md5(uint8_t *result);
	uint16_t getErrorCode() const;
	bool hasError() const;
	void clearError();
	const char *errorString() const;
	void printError(Print &out) const;

  private: /* UpdateUtil.cpp */
	static void progressHandler(UpdateClass *self);
	void printErrorContext();

  private:
	lt_ota_ctx_t *ctx{nullptr};
	uf2_err_t errUf2{UF2_ERR_OK};
	UpdateError errArd{UPDATE_ERROR_OK};
	THandlerFunction_Progress callback{nullptr};
	LT_MD5_CTX_T *md5Ctx{nullptr};
	uint8_t *md5Digest{nullptr};
	uint8_t *md5Expected{nullptr};

  public:
	/**
	 * @brief Get Arduino error code of the update.
	 */
	inline UpdateError getError() const {
		return this->errArd;
	}

	/**
	 * @brief Get UF2OTA error code of the update.
	 */
	inline uf2_err_t getUF2Error() const {
		return this->ctx ? this->ctx->error : this->errUf2;
	}

	/**
	 * @brief Same as end().
	 */
	inline void abort() {
		this->end();
	}

	/**
	 * @brief Check if the update process has been started.
	 */
	inline bool isRunning() {
		return this->ctx;
	}

	/**
	 * @brief Check if the update process hasn't been started or has been completed.
	 */
	inline bool isFinished() {
		return !(this->ctx && this->ctx->bytes_written != this->ctx->bytes_total);
	}

	/**
	 * @brief Return complete update image size.
	 */
	inline size_t size() {
		return this->ctx ? this->ctx->bytes_total : 0;
	}

	/**
	 * @brief Return amount of bytes already written.
	 */
	inline size_t progress() {
		return this->ctx ? this->ctx->bytes_written : 0;
	}

	/**
	 * @brief Return amount of bytes remaining to write.
	 */
	inline size_t remaining() {
		return this->size() - this->progress();
	}

	/**
	 * @brief Get firmware name from UF2 info.
	 */
	inline const char *getFirmwareName() {
		if (this->ctx)
			return this->ctx->info.fw_name;
		return nullptr;
	}

	/**
	 * @brief Get firmware version from UF2 info.
	 */
	inline const char *getFirmwareVersion() {
		if (this->ctx)
			return this->ctx->info.fw_version;
		return nullptr;
	}

	/**
	 * @brief Get LibreTiny version from UF2 info.
	 */
	inline const char *getLibreTinyVersion() {
		if (this->ctx)
			return this->ctx->info.lt_version;
		return nullptr;
	}

	/**
	 * @brief Get target board name from UF2 info.
	 */
	inline const char *getBoardName() {
		if (this->ctx)
			return this->ctx->info.board;
		return nullptr;
	}
};

extern UpdateClass Update;

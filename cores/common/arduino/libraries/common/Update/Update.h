#pragma once

#include <Arduino.h>
#include <functional>
#include <uf2ota/uf2ota.h>

// No Error
#define UPDATE_ERROR_OK			  (0)
// Flash Write Failed
#define UPDATE_ERROR_WRITE		  (1)
// Flash Erase Failed
#define UPDATE_ERROR_ERASE		  (2)
// Flash Read Failed
#define UPDATE_ERROR_READ		  (3)
// Not Enough Space
#define UPDATE_ERROR_SPACE		  (4)
// Bad Size Given
#define UPDATE_ERROR_SIZE		  (5)
// Stream Read Timeout
#define UPDATE_ERROR_STREAM		  (6)
// MD5 Check Failed
#define UPDATE_ERROR_MD5		  (7)
// Wrong Magic Byte
#define UPDATE_ERROR_MAGIC_BYTE	  (8)
// Could Not Activate The Firmware
#define UPDATE_ERROR_ACTIVATE	  (9)
// Partition Could Not be Found
#define UPDATE_ERROR_NO_PARTITION (10)
// Bad Argument
#define UPDATE_ERROR_BAD_ARGUMENT (11)
// Aborted
#define UPDATE_ERROR_ABORT		  (12)

#define UPDATE_SIZE_UNKNOWN 0xFFFFFFFF

#define U_FLASH	 0
#define U_SPIFFS 100
#define U_AUTH	 200

#define ENCRYPTED_BLOCK_SIZE 16

#define UPDATE_TIMEOUT_MS 30 * 1000

class UpdateClass {
  public:
	typedef std::function<void(size_t, size_t)> THandlerFunction_Progress;

  public: /* Update.cpp */
	UpdateClass();
	bool begin(
		size_t size			= UPDATE_SIZE_UNKNOWN,
		int command			= U_FLASH,
		int unused2			= -1,
		uint8_t unused3		= LOW,
		const char *unused4 = NULL // this is for SPIFFS
	);
	bool end(bool evenIfRemaining = false);
	size_t write(uint8_t *data, size_t len);
	size_t writeStream(Stream &data);
	bool canRollBack();
	bool rollBack();
	// bool setMD5(const char *expected_md5);

  private: /* Update.cpp */
	size_t tryWriteData(uint8_t *data = NULL, size_t len = 0);

  public: /* UpdateUtil.cpp */
	UpdateClass &onProgress(THandlerFunction_Progress callback);
	void abort();
	void printError(Print &out);
	const char *errorString();
	const char *getFirmwareName();
	const char *getFirmwareVersion();
	const char *getLibreTuyaVersion();
	const char *getBoardName();

  private: /* UpdateUtil.cpp */
	void cleanup(uint8_t ardErr = UPDATE_ERROR_OK, uf2_err_t uf2Err = UF2_ERR_OK);
	bool checkUf2Error(uf2_err_t err);
	void bufAlloc();
	void printErrorContext1();
	void printErrorContext2(const uint8_t *data, size_t len);
	uint16_t bufLeft();
	uint16_t bufSize();

  private:
	// uf2ota context
	uf2_ota_t *ctx;
	uf2_info_t *info;
	// block buffer
	uint8_t *buf;
	uint8_t *bufPos;
	// update progress - multiplies of 512 bytes
	uint32_t bytesWritten;
	uint32_t bytesTotal;
	// errors
	uf2_err_t errUf2;
	uint8_t errArd;
	// progress callback
	THandlerFunction_Progress callback;
	// String _target_md5;
	// MD5Builder _md5;

  public:
	String md5String(void) {
		// return _md5.toString();
	}

	void md5(uint8_t *result) {
		// return _md5.getBytes(result);
	}

	uint8_t getError() {
		return errArd;
	}

	uf2_err_t getUF2Error() {
		return errUf2;
	}

	uint16_t getErrorCode() {
		return (errArd << 8) | errUf2;
	}

	void clearError() {
		cleanup(UPDATE_ERROR_OK);
	}

	bool hasError() {
		return errArd != UPDATE_ERROR_OK;
	}

	bool isRunning() {
		return ctx != NULL;
	}

	bool isFinished() {
		return bytesWritten == bytesTotal;
	}

	size_t size() {
		return bytesTotal;
	}

	size_t progress() {
		return bytesWritten;
	}

	size_t remaining() {
		return bytesTotal - bytesWritten;
	}
};

extern UpdateClass Update;

/* Copyright (c) Kuba Szczodrzyński 2022-04-28. */

#pragma once

// LibreTuya version macros
#ifndef LT_VERSION
#define LT_VERSION 1.0.0
#endif
#ifndef LT_BOARD
#define LT_BOARD unknown
#endif
#define STRINGIFY(x)	   #x
#define STRINGIFY_MACRO(x) STRINGIFY(x)
#define LT_VERSION_STR	   STRINGIFY_MACRO(LT_VERSION)
#define LT_BOARD_STR	   STRINGIFY_MACRO(LT_BOARD)

// Includes
#include "LibreTuyaConfig.h"
#include <Arduino.h>

// C includes
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "lt_logger.h"
#include "lt_posix_api.h"

#ifdef __cplusplus
} // extern "C"
#endif

// Functional macros
#define LT_BANNER()                                                                                                    \
	LT_LOG(                                                                                                            \
		LT_LEVEL_INFO,                                                                                                 \
		__FUNCTION__,                                                                                                  \
		__LINE__,                                                                                                      \
		"LibreTuya v" LT_VERSION_STR " on " LT_BOARD_STR ", compiled at " __DATE__ " " __TIME__                        \
	)

// ArduinoCore-API doesn't define these anymore
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#define PGM_VOID_P			const void *

void lt_rand_bytes(uint8_t *buf, size_t len);

#ifdef __cplusplus
String ipToString(const IPAddress &ip);
void hexdump(uint8_t *buf, size_t len, uint32_t offset = 0, uint8_t width = 16);
#else
void hexdump(uint8_t *buf, size_t len, uint32_t offset, uint8_t width);
#endif

// Main class
#ifdef __cplusplus

#include <Flash.h> // for flash inline methods
#include <core/ChipType.h>

/**
 * @brief Main LibreTuya API class.
 *
 * This class contains all functions common amongst all families.
 * Implementations of these methods may vary between families.
 *
 * The class is accessible using the `LT` global object (defined by the family).
 */
class LibreTuya {
  public: /* Common methods - note: these are documented in LibreTuyaAPI.cpp */
	const char *getVersion();
	const char *getBoard();
	ChipFamily getChipFamily();
	const char *getChipFamilyName();
	const char *getDeviceName();
	uint8_t otaGetRunning();
	uint8_t otaGetTarget();
	bool otaRollback();
	bool otaCanRollback();

  public: /* Inline methods */
	inline uint32_t getFlashChipSize() {
		return Flash.getSize();
	}

	// inline bool flashEraseSector(uint32_t sector) {}
	// inline bool flashWrite(uint32_t offset, uint32_t *data, size_t size) {}
	// inline bool flashRead(uint32_t offset, uint32_t *data, size_t size) {}
	// inline bool partitionEraseRange(const esp_partition_t *partition, uint32_t offset, size_t size) {}
	// inline bool partitionWrite(const esp_partition_t *partition, uint32_t offset, uint32_t *data, size_t size) {}
	// inline bool partitionRead(const esp_partition_t *partition, uint32_t offset, uint32_t *data, size_t size) {}

  public: /* Family-defined methods */
	/**
	 * @brief Reboot the CPU.
	 */
	void restart();

  public: /* CPU-related */
	/**
	 * @brief Get CPU model ID.
	 */
	ChipType getChipType();
	/**
	 * @brief Get CPU model name as string.
	 */
	const char *getChipModel();
	/**
	 * @brief Get CPU unique ID. This may be based on MAC, eFuse, etc.
	 */
	uint32_t getChipId();
	/**
	 * @brief Get CPU core count.
	 */
	uint8_t getChipCores();
	/**
	 * @brief Get CPU core type name as string.
	 */
	const char *getChipCoreType();
	/**
	 * @brief Get CPU frequency in MHz.
	 */
	uint32_t getCpuFreqMHz();
	/**
	 * @brief Get CPU cycle count.
	 */
	inline uint32_t getCycleCount() __attribute__((always_inline));

  public: /* Memory management */
	/**
	 * @brief Get total RAM size.
	 */
	uint32_t getRamSize();
	/**
	 * @brief Get total heap size.
	 */
	uint32_t getHeapSize();
	/**
	 * @brief Get free heap size.
	 */
	uint32_t getFreeHeap();
	/**
	 * @brief Get lowest level of free heap memory.
	 */
	uint32_t getMinFreeHeap();
	/**
	 * @brief Get largest block of heap that can be allocated at once.
	 */
	uint32_t getMaxAllocHeap();

  public: /* OTA-related */
	/**
	 * @brief Read the currently active OTA index, i.e. the one that will boot upon restart.
	 */
	uint8_t otaGetStoredIndex();
	/**
	 * @brief Check if the chip supports dual-OTA.
	 */
	bool otaSupportsDual();
	/**
	 * @brief Check if OTA1 image is valid.
	 */
	bool otaHasImage1();
	/**
	 * @brief Check if OTA2 image is valid.
	 */
	bool otaHasImage2();
	/**
	 * @brief Try to switch OTA index to the other image.
	 *
	 * Note: should return true for chips without dual-OTA. Should return false if one of two images is not valid.
	 *
	 * @param force switch even if other image already marked as active
	 * @return false if writing failed; true otherwise
	 */
	bool otaSwitch(bool force = false);
};

extern LibreTuya LT;
extern LibreTuya ESP;
#endif

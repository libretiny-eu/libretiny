/* Copyright (c) Kuba Szczodrzyński 2023-03-20.() */

#include <Arduino.h>

#ifdef __cplusplus

/**
 * @brief ESP Arduino Core compatibility class.
 *
 * This class only consists of inline functions, which
 * wrap the LibreTiny C API (lt_api.h). Refer to the docs of the C API
 * for more information.
 *
 * The class is accessible using the `ESP` global object.
 */
class EspClass {
  public:
	/** @copydoc lt_wdt_enable() */
	inline void wdtEnable(uint32_t timeout_ms = 0) { lt_wdt_enable(timeout_ms); }

	/** @copydoc lt_wdt_disable() */
	inline void wdtDisable() { lt_wdt_disable(); }

	/** @copydoc lt_wdt_feed() */
	inline void wdtFeed() { lt_wdt_feed(); }

	/** @copydoc lt_reboot() */
	inline void reset() { lt_reboot(); }

	/** @copydoc lt_reboot() */
	inline void restart() { lt_reboot(); }

	/** @copydoc lt_reboot_download_mode() */
	inline void rebootIntoUartDownloadMode() { lt_reboot_download_mode(); }

	inline uint16_t getVcc() { return 3300; }

	/** @copydoc lt_cpu_get_mac_id() */
	inline uint32_t getChipId() { return lt_cpu_get_mac_id(); }

	/** @copydoc lt_heap_get_free() */
	inline uint32_t getFreeHeap() { return lt_heap_get_free(); }

	/** @copydoc lt_heap_get_max_alloc() */
	inline uint16_t getMaxFreeBlockSize() { return lt_heap_get_max_alloc(); }

	/** @copydoc LT_VERSION_STR() */
	inline const char *getSdkVersion() { return LT_VERSION_STR; }

	/** @copydoc LT_VERSION_STR() */
	inline String getCoreVersion() { return LT_VERSION_STR; }

	/** @copydoc LT_BANNER_STR() */
	inline String getFullVersion() { return LT_BANNER_STR; }

	inline uint8_t getBootVersion() { return 0; }

	inline uint8_t getBootMode() { return 0; }

	/** @copydoc lt_cpu_get_freq_mhz() */
	inline uint8_t getCpuFreqMHz() { return lt_cpu_get_freq_mhz(); }

	/** @copydoc lt_flash_get_id() */
	inline uint32_t getFlashChipId() {
		lt_flash_id_t id = lt_flash_get_id();
		return id.manufacturer_id << 16 | id.chip_id << 7 | id.chip_size_id;
	}

	/** @copydoc lt_flash_get_id() */
	inline uint8_t getFlashChipVendorId() { return lt_flash_get_id().manufacturer_id; }

	/** @copydoc lt_flash_get_size() */
	inline uint32_t getFlashChipRealSize() { return lt_flash_get_size(); }

	/** @copydoc lt_flash_get_size() */
	inline uint32_t getFlashChipSize() { return lt_flash_get_size(); }

	inline uint32_t getFlashChipMode() { return 0xFF; }

	/** @copydoc lt_flash_get_size() */
	inline uint32_t getFlashChipSizeByChipId() { return lt_flash_get_size(); }

	/** @copydoc lt_flash_erase_block() */
	inline bool flashEraseSector(uint32_t sector) { return lt_flash_erase_block(sector); }

	/** @copydoc lt_flash_write() */
	inline bool flashWrite(uint32_t address, const uint8_t *data, size_t size) {
		return lt_flash_write(address, data, size) == size;
	}

	/** @copydoc lt_flash_read() */
	inline bool flashRead(uint32_t address, uint8_t *data, size_t size) {
		return lt_flash_read(address, data, size) == size;
	}

	/** @copydoc lt_get_reboot_reason_name() */
	inline String getResetReason() { return lt_get_reboot_reason_name(lt_get_reboot_reason()); }

	/** @copydoc lt_get_reboot_reason_name() */
	inline String getResetInfo() { return lt_get_reboot_reason_name(lt_get_reboot_reason()); }

	/** @copydoc lt_rand_bytes() */
	inline uint8_t *random(uint8_t *resultArray, const size_t outputSizeBytes) {
		lt_rand_bytes(resultArray, (size_t)outputSizeBytes);
		return resultArray;
	}

	/** @copydoc lt_rand_bytes() */
	inline uint32_t random() {
		uint32_t i;
		lt_rand_bytes((uint8_t *)&i, 4);
		return i;
	}

	/** @copydoc lt_cpu_get_cycle_count() */
	inline uint32_t getCycleCount() { return lt_cpu_get_cycle_count(); }
};

extern EspClass ESP;

#endif

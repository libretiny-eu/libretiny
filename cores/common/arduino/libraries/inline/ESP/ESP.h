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
	/** 
	 * @brief Enable the hardware watchdog.
	 *
	 * @param timeout watchdog timeout, milliseconds
	 * @return whether the chip has a hardware watchdog
	  */
	inline void wdtEnable(uint32_t timeout_ms = 0) { lt_wdt_enable(timeout_ms); }

	/** 
	 * @brief Disable the hardware watchdog.
	  */
	inline void wdtDisable() { lt_wdt_disable(); }

	/** 
	 * @brief Feed/reset the hardware watchdog timer.
	  */
	inline void wdtFeed() { lt_wdt_feed(); }

	/** 
	 * @brief Reboot the CPU.
	  */
	inline void reset() { lt_reboot(); }

	/** 
	 * @brief Reboot the CPU.
	  */
	inline void restart() { lt_reboot(); }

	/** 
	 * @brief Reboot the CPU and stay in download mode (if possible).
	 *
	 * @return whether download-mode reboot is possible
	  */
	inline void rebootIntoUartDownloadMode() { lt_reboot_download_mode(); }

	inline uint16_t getVcc() { return 3300; }

	/** 
	 * @brief Get CPU ID based on the last three octets of MAC address.
	 * Note: the number is 24-bit (with the MSB being zero).
	 * The 3rd-to-last octet is least-significant, the last octet is most-significant.
	  */
	inline uint32_t getChipId() { return lt_cpu_get_mac_id(); }

	/** 
	 * @brief Get free heap size.
	  */
	inline uint32_t getFreeHeap() { return lt_heap_get_free(); }

	/** 
	 * @brief Get largest block of heap that can be allocated at once.
	  */
	inline uint16_t getMaxFreeBlockSize() { return lt_heap_get_max_alloc(); }

	/** @copydoc LT_VERSION_STR() */
	inline const char *getSdkVersion() { return LT_VERSION_STR; }

	/** @copydoc LT_VERSION_STR() */
	inline String getCoreVersion() { return LT_VERSION_STR; }

	/** @copydoc LT_BANNER_STR() */
	inline String getFullVersion() { return LT_BANNER_STR; }

	inline uint8_t getBootVersion() { return 0; }

	inline uint8_t getBootMode() { return 0; }

	/** 
	 * @brief Get CPU frequency in MHz.
	  */
	inline uint8_t getCpuFreqMHz() { return lt_cpu_get_freq_mhz(); }

	/** 
	 * @brief Flash chip ID structure.
	  */
	inline uint32_t getFlashChipId() {
		lt_flash_id_t id = lt_flash_get_id();
		return id.manufacturer_id << 16 | id.chip_id << 7 | id.chip_size_id;
	}

	/** 
	 * @brief Flash chip ID structure.
	  */
	inline uint8_t getFlashChipVendorId() { return lt_flash_get_id().manufacturer_id; }

	/** 
	 * @brief Get flash chip total size.
	 *
	 * The default implementation uses the least significant
	 * byte of the chip ID to determine the size.
	  */
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

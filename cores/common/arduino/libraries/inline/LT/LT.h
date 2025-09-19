/* Copyright (c) Kuba Szczodrzyński 2022-06-06. */

#pragma once

#include <Arduino.h>
#include <ESP.h>
#include <OTA.h>
#include <WDT.h>

#ifdef __cplusplus

#define ChipFamily	lt_cpu_family_t
#define ChipType	lt_cpu_model_t
#define ResetReason lt_reboot_reason_t
#define FlashId		lt_flash_id_t

/**
 * @brief Main LibreTiny API class.
 *
 * Since v1.0.0, this class only consists of inline functions, which
 * wrap the LibreTiny C API (lt_api.h). Refer to the docs of the C API
 * for more information.
 *
 * The class is accessible using the `LT` global object.
 */
class LibreTiny {
  public: /* lt_cpu.h */
	/** 
	 * @brief Get CPU family ID (as lt_cpu_family_t enum member).
	  */
	inline ChipFamily getChipFamily() { return lt_cpu_get_family(); }

	/** 
	 * @brief Get CPU family name as string.
	  */
	inline const char *getChipFamilyName() { return lt_cpu_get_family_name(); }

	/** 
	 * @brief Get CPU model ID (as lt_cpu_model_t enum member).
	  */
	inline ChipType getChipType() { return lt_cpu_get_model(); }

	/** 
	 * @brief Get CPU model name as string (uppercase).
	  */
	inline const char *getChipModel() { return lt_cpu_get_model_name(); }

	/** 
	 * @brief Get CPU ID based on the last three octets of MAC address.
	 * Note: the number is 24-bit (with the MSB being zero).
	 * The 3rd-to-last octet is least-significant, the last octet is most-significant.
	  */
	inline uint32_t getChipId() { return lt_cpu_get_mac_id(); }

	/** 
	 * @brief Get CPU core count.
	  */
	inline uint8_t getChipCores() { return lt_cpu_get_core_count(); }

	/** 
	 * @brief Get CPU core type name as string.
	  */
	inline const char *getChipCoreType() { return lt_cpu_get_core_type(); }

	/** 
	 * @brief Get CPU frequency in Hz.
	  */
	inline uint32_t getCpuFreq() { return lt_cpu_get_freq(); }

	/** 
	 * @brief Get CPU frequency in MHz.
	  */
	inline uint32_t getCpuFreqMHz() { return lt_cpu_get_freq_mhz(); }

	/** 
	 * @brief Get CPU cycle count.
	  */
	inline uint32_t getCycleCount() { return lt_cpu_get_cycle_count(); }

  public: /* lt_device.h */
	/** 
	 * @brief Reset reason enumeration.
	  */
	inline const char *getVersion() { return lt_get_version(); }

	/** 
	 * @brief Get board code.
	  */
	inline const char *getBoard() { return lt_get_board_code(); }

	/** 
	 * @brief Get device friendly name in format "LT-<chip model>-<MAC ID>".
	 * Can be used as hostname.
	  */
	inline const char *getDeviceName() { return lt_get_device_name(); }

	/** 
	 * @brief Reboot the CPU.
	  */
	inline void restart() { lt_reboot(); }

	/** 
	 * @brief Reboot the CPU and stay in download mode (if possible).
	 *
	 * @return whether download-mode reboot is possible
	  */
	inline void restartDownloadMode() { lt_reboot_download_mode(); }

	/** 
	 * @brief Get the reason of last chip reboot.
	  */
	inline ResetReason getResetReason() { return lt_get_reboot_reason(); }

	/** @copydoc lt_get_reboot_reason_name() */
	inline const char *getResetReasonName(ResetReason reason = lt_get_reboot_reason()) {
		return lt_get_reboot_reason_name(reason);
	}

	/** @copydoc lt_gpio_recover(); */
	inline void gpioRecover() { lt_gpio_recover(); }

  public: /* lt_flash.h */
	/** @copydoc lt_flash_get_id() */
	inline FlashId getFlashChipId() { return lt_flash_get_id(); }

	/** @copydoc lt_flash_get_size() */
	inline uint32_t getFlashChipSize() { return lt_flash_get_size(); }

  public: /* lt_mem.h */
	/** @copydoc lt_ram_get_size() */
	inline uint32_t getRamSize() { return lt_ram_get_size(); }

	/** @copydoc lt_heap_get_size() */
	inline uint32_t getHeapSize() { return lt_heap_get_size(); }

	/** @copydoc lt_heap_get_free() */
	inline uint32_t getFreeHeap() { return lt_heap_get_free(); }

	/** @copydoc lt_heap_get_min_free() */
	inline uint32_t getMinFreeHeap() { return lt_heap_get_min_free(); }

	/** @copydoc lt_heap_get_max_alloc() */
	inline uint32_t getMaxAllocHeap() { return lt_heap_get_max_alloc(); }

	/** @copydoc lt_heap_get_max_alloc() */
	inline uint32_t getMaxFreeBlockSize() { return lt_heap_get_max_alloc(); }
};

extern LibreTiny LT;

#endif

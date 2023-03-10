/* Copyright (c) Kuba Szczodrzyński 2022-06-06. */

#pragma once

#include <Arduino.h>
#include <OTA.h>
#include <WDT.h>

#ifdef __cplusplus

#define ChipFamily	lt_cpu_family_t
#define ChipType	lt_cpu_model_t
#define ResetReason lt_reboot_reason_t
#define FlashId		lt_flash_id_t

/**
 * @brief Main LibreTuya API class.
 *
 * Since v1.0.0, this class only consists of inline functions, which
 * wrap the LibreTuya C API (lt_api.h). Refer to the docs of the C API
 * for more information.
 *
 * The class is accessible using the `LT` global object.
 */
class LibreTuya {
  public: /* lt_cpu.h */
	/** @copydoc lt_get_cpu_family() */
	inline ChipFamily getChipFamily() { return lt_get_cpu_family(); }

	/** @copydoc lt_get_cpu_family_name() */
	inline const char *getChipFamilyName() { return lt_get_cpu_family_name(); }

	/** @copydoc lt_get_cpu_model() */
	inline ChipType getChipType() { return lt_get_cpu_model(); }

	/** @copydoc lt_get_cpu_model_name() */
	inline const char *getChipModel() { return lt_get_cpu_model_name(); }

	/** @copydoc lt_get_cpu_mac_id() */
	inline uint32_t getChipId() { return lt_get_cpu_mac_id(); }

	/** @copydoc lt_get_cpu_core_count() */
	inline uint8_t getChipCores() { return lt_get_cpu_core_count(); }

	/** @copydoc lt_get_cpu_core_type() */
	inline const char *getChipCoreType() { return lt_get_cpu_core_type(); }

	/** @copydoc lt_get_cpu_freq() */
	inline uint32_t getCpuFreq() { return lt_get_cpu_freq(); }

	/** @copydoc lt_get_cpu_freq_mhz() */
	inline uint32_t getCpuFreqMHz() { return lt_get_cpu_freq_mhz(); }

	/** @copydoc lt_get_cpu_cycle_count() */
	inline uint32_t getCycleCount() { return lt_get_cpu_cycle_count(); }

  public: /* lt_device.h */
	/** @copydoc lt_get_version() */
	inline const char *getVersion() { return lt_get_version(); }

	/** @copydoc lt_get_board_code() */
	inline const char *getBoard() { return lt_get_board_code(); }

	/** @copydoc lt_get_device_name() */
	inline const char *getDeviceName() { return lt_get_device_name(); }

	/** @copydoc lt_reboot() */
	inline void restart() { lt_reboot(); }

	/** @copydoc lt_reboot_download_mode() */
	inline void restartDownloadMode() { lt_reboot_download_mode(); }

	/** @copydoc lt_get_reboot_reason() */
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
	/** @copydoc lt_get_ram_size() */
	inline uint32_t getRamSize() { return lt_get_ram_size(); }

	/** @copydoc lt_get_heap_size() */
	inline uint32_t getHeapSize() { return lt_get_heap_size(); }

	/** @copydoc lt_get_heap_free() */
	inline uint32_t getFreeHeap() { return lt_get_heap_free(); }

	/** @copydoc lt_get_heap_min_free() */
	inline uint32_t getMinFreeHeap() { return lt_get_heap_min_free(); }

	/** @copydoc lt_get_heap_max_alloc() */
	inline uint32_t getMaxAllocHeap() { return lt_get_heap_max_alloc(); }

	/** @copydoc lt_get_heap_max_alloc() */
	inline uint32_t getMaxFreeBlockSize() { return lt_get_heap_max_alloc(); }
};

extern LibreTuya LT;

#endif

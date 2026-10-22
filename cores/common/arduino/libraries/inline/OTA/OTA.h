/* Copyright (c) Kuba Szczodrzyński 2023-03-10. */

#pragma once

#include <Arduino.h>

#ifdef __cplusplus

/**
 * @brief Over-the-Air updates helper class.
 *
 * This class only consists of inline functions, which
 * wrap the LibreTiny C API (lt_api.h). Refer to the docs of the C API
 * for more information.
 *
 * The class is accessible using the `OTA` global object.
 */
class LibreTinyOTA {
  public: /* lt_ota.h */
	/** 
	 * @brief Get OTA type of the device's chip.
	  */
	inline lt_ota_type_t getType() { return lt_ota_get_type(); }

	/** 
	 * @brief Check if the specified OTA image is valid.
	 *
	 * @param index OTA index to check; 0 for single-OTA chips, 1 or 2 for dual-OTA chips
	 * @return true if index is valid for the chip's OTA type, and there is a valid image; false otherwise
	  */
	inline bool isValid(uint8_t index) { return lt_ota_is_valid(index); }

	/** 
	 * @brief Check if OTA rollback is possible (switching the stored index to another partition).
	 *
	 * Note that this is not the same as "switching" OTA with revert=true.
	 *
	 * @return true if 2nd image is valid and the chip is dual-OTA; false otherwise
	  */
	inline bool canRollback() { return lt_ota_can_rollback(); }

	/** 
	 * @brief Get the currently running firmware's OTA index.
	 *
	 * @return OTA index if dual-OTA is supported, 0 otherwise
	  */
	inline uint8_t getCurrentIndex() { return lt_ota_dual_get_current(); }

	/** 
	 * @brief Read the currently active OTA index, i.e. the one that will boot upon restart.
	 *
	 * @return OTA index if dual-OTA is supported, 0 otherwise
	  */
	inline uint8_t getStoredIndex() { return lt_ota_dual_get_stored(); }

	/** 
	 * @brief Check which UF2 OTA scheme should be used for applying firmware updates.
	 *
	 * @return OTA scheme of the target partition
	  */
	inline uf2_ota_scheme_t getUF2Scheme() { return lt_ota_get_uf2_scheme(); }

	/** 
	 * @brief Try to switch OTA index to the other image. For single-OTA chips, only check if the upgrade image is valid.
	 *
	 * This can be used to "activate" the upgrade after flashing.
	 *
	 * @param revert switch if (and only if) the other image is already marked as active (i.e.
	 * switch back to the running image)
	 * @return false if the second image (or upgrade image) is not valid; false if writing failed; true otherwise
	  */
	inline bool switchImage(bool revert = false) { return lt_ota_switch(revert); }
};

extern LibreTinyOTA OTA;

#endif

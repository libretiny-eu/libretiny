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
	/** @copydoc lt_ota_get_type() */
	inline lt_ota_type_t getType() { return lt_ota_get_type(); }

	/** @copydoc lt_ota_is_valid() */
	inline bool isValid(uint8_t index) { return lt_ota_is_valid(index); }

	/** @copydoc lt_ota_can_rollback() */
	inline bool canRollback() { return lt_ota_can_rollback(); }

	/** @copydoc lt_ota_dual_get_current() */
	inline uint8_t getCurrentIndex() { return lt_ota_dual_get_current(); }

	/** @copydoc lt_ota_dual_get_stored() */
	inline uint8_t getStoredIndex() { return lt_ota_dual_get_stored(); }

	/** @copydoc lt_ota_get_uf2_scheme() */
	inline uf2_ota_scheme_t getUF2Scheme() { return lt_ota_get_uf2_scheme(); }

	/** @copydoc lt_ota_switch() */
	inline bool switchImage(bool revert = false) { return lt_ota_switch(revert); }
};

extern LibreTinyOTA OTA;

#endif

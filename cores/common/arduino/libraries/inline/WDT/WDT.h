/* Copyright (c) Kuba Szczodrzyński 2023-03-10. */

#pragma once

#include <Arduino.h>

#ifdef __cplusplus

/**
 * @brief Watchdog control class.
 *
 * This class only consists of inline functions, which
 * wrap the LibreTiny C API (lt_api.h). Refer to the docs of the C API
 * for more information.
 *
 * The class is accessible using the `WDT` global object.
 */
class LibreTinyWDT {
  public: /* lt_wdt.h */
	/** 
	 * @brief Enable the hardware watchdog.
	 *
	 * @param timeout watchdog timeout, milliseconds
	 * @return whether the chip has a hardware watchdog
	  */
	inline bool enable(uint32_t timeout = 10000) { return lt_wdt_enable(timeout); }

	/** 
	 * @brief Disable the hardware watchdog.
	  */
	inline void disable() { lt_wdt_disable(); }

	/** 
	 * @brief Feed/reset the hardware watchdog timer.
	  */
	inline void feed() { lt_wdt_feed(); }
};

extern LibreTinyWDT WDT;

#endif

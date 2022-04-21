/** @file wiring_watchdog.h */

/** 
 * @defgroup wiring_watchdog wiring_watchdog
 * watchdog is used for reboot system when device hang
 * @{
 */

#ifndef _WIRING_WATCHDOG_H_
#define _WIRING_WATCHDOG_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup watchdog */

/**
 * @ingroup watchdog
 * Feed watchdog to avoid it barks
 */
extern void wdt_reset();

/**
 * @ingroup watchdog
 * @brief Enable watchdog
 *
 * After enbling watchdog, user defined code needs to reset it before watchdog barks. Otherwise it would make system reboot.
 *
 * @param[in] timeout_ms The timeout value that watchdog barks. The maximum value is 8 seconds.
 */
extern void wdt_enable(uint32_t timeout_ms);

/**
 * @ingroup watchdog
 * Disable watchdog
 */
extern void wdt_disable();

#ifdef __cplusplus
}
#endif

#endif /* _WIRING_WATCHDOG_H_ */

/** @} */ // end of group wiring_watchdog
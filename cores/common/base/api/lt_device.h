/* Copyright (c) Kuba Szczodrzyński 2023-03-09. */

#pragma once

#include <libretiny.h>

#define RESET_REASON_UNKNOWN	REBOOT_REASON_UNKNOWN
#define RESET_REASON_POWER		REBOOT_REASON_POWER
#define RESET_REASON_BROWNOUT	REBOOT_REASON_BROWNOUT
#define RESET_REASON_HARDWARE	REBOOT_REASON_HARDWARE
#define RESET_REASON_SOFTWARE	REBOOT_REASON_SOFTWARE
#define RESET_REASON_WATCHDOG	REBOOT_REASON_WATCHDOG
#define RESET_REASON_CRASH		REBOOT_REASON_CRASH
#define RESET_REASON_SLEEP_GPIO REBOOT_REASON_SLEEP_GPIO
#define RESET_REASON_SLEEP_RTC	REBOOT_REASON_SLEEP_RTC
#define RESET_REASON_SLEEP_USB	REBOOT_REASON_SLEEP_USB
#define RESET_REASON_MAX		REBOOT_REASON_MAX

/**
 * @brief Reset reason enumeration.
 */
typedef enum {
	REBOOT_REASON_UNKNOWN	 = 1,
	REBOOT_REASON_POWER		 = 2,
	REBOOT_REASON_BROWNOUT	 = 3,
	REBOOT_REASON_HARDWARE	 = 4,
	REBOOT_REASON_SOFTWARE	 = 5,
	REBOOT_REASON_WATCHDOG	 = 6,
	REBOOT_REASON_CRASH		 = 7,
	REBOOT_REASON_SLEEP_GPIO = 8,
	REBOOT_REASON_SLEEP_RTC	 = 9,
	REBOOT_REASON_SLEEP_USB	 = 10,
	REBOOT_REASON_DEBUGGER	 = 11,
	REBOOT_REASON_MAX		 = 12,
} lt_reboot_reason_t;

// RESET_REASON_SLEEP deprecated, kept for compatibility
#define RESET_REASON_SLEEP	REBOOT_REASON_SLEEP_GPIO
#define REBOOT_REASON_SLEEP REBOOT_REASON_SLEEP_GPIO

/**
 * @brief Debugging mode enumeration.
 */
typedef enum {
	DEBUG_MODE_OFF	= 0,
	DEBUG_MODE_JTAG = 1,
	DEBUG_MODE_SWD	= 2,
} lt_debug_mode_t;

/**
 * @brief Get LibreTiny version string.
 */
const char *lt_get_version();

/**
 * @brief Get board code.
 */
const char *lt_get_board_code();

/**
 * @brief Get device friendly name in format "LT-<chip model>-<MAC ID>".
 * Can be used as hostname.
 */
const char *lt_get_device_name();

/**
 * @brief Read device's *default* MAC address into 'mac' array.
 * This can be used even without Wi-Fi enabled, and will ignore
 * user-changed Wi-Fi MAC (if changing is possible).
 */
void lt_get_device_mac(uint8_t *mac);

/**
 * @brief Reboot the CPU.
 */
void lt_reboot();

/**
 * @brief Reboot the CPU with a watchdog timeout (if possible).
 *
 * @return whether WDT reboot is possible
 */
bool lt_reboot_wdt();

/**
 * @brief Reboot the CPU and stay in download mode (if possible).
 *
 * @return whether download-mode reboot is possible
 */
bool lt_reboot_download_mode();

/**
 * @brief Get the reason of last chip reboot.
 */
lt_reboot_reason_t lt_get_reboot_reason();

/**
 * @brief Get a textual representation of a reboot reason.
 *
 * @param reason value to convert to text, pass 0 to read from lt_reboot_get_reason()
 */
const char *lt_get_reboot_reason_name(lt_reboot_reason_t reason);

/**
 * @brief Set debugger mode (JTAG, SWD or OFF).
 *
 * @return whether the mode is supported, and setting it was successful
 */
bool lt_set_debug_mode(lt_debug_mode_t mode);

/**
 * @brief Reconfigure GPIO pins used for debugging
 * (SWD/JTAG), so that they can be used as normal I/O.
 */
void lt_gpio_recover();

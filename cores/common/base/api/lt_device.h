/* Copyright (c) Kuba Szczodrzyński 2023-03-09. */

#pragma once

#include <libretiny.h>

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
 * @brief Reconfigure GPIO pins used for debugging
 * (SWD/JTAG), so that they can be used as normal I/O.
 */
void lt_gpio_recover();

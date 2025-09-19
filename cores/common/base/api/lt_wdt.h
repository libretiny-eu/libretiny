/* Copyright (c) Kuba Szczodrzyński 2023-03-09. */

#pragma once

#include <libretiny.h>

/**
 * @brief Enable the hardware watchdog.
 *
 * @param timeout watchdog timeout, milliseconds
 * @return whether the chip has a hardware watchdog
 */
bool lt_wdt_enable(uint32_t timeout);

/**
 * @brief Disable the hardware watchdog.
 */
void lt_wdt_disable();

/**
 * @brief Feed/reset the hardware watchdog timer.
 */
void lt_wdt_feed();

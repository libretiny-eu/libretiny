/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#pragma once

#include <libretiny.h>

/**
 * @brief Initialize the family core (optional).
 * This method is family-specific; the family core can do whatever it wants to.
 * This method is empty if not implemented, and shouldn't be called manually.
 */
void lt_init_family() __attribute__((weak));

/**
 * @brief Initialize the board (variant).
 * This method is empty if not implemented (which is usually the case),
 * and shouldn't be called manually.
 */
void lt_init_variant() __attribute__((weak));

/**
 * @brief Initialize the family's Arduino core (optional).
 * This method is family-specific; the family core can do whatever it wants to.
 * This method is empty if not implemented, and shouldn't be called manually.
 */
void lt_init_arduino() __attribute__((weak));

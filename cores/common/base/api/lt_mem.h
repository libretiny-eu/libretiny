/* Copyright (c) Kuba Szczodrzyński 2023-03-09. */

#pragma once

#include <libretuya.h>

/**
 * @brief Get total RAM size.
 */
uint32_t lt_get_ram_size();

/**
 * @brief Get total heap size.
 */
uint32_t lt_get_heap_size();

/**
 * @brief Get free heap size.
 */
uint32_t lt_get_heap_free();

/**
 * @brief Get lowest level of free heap memory.
 */
uint32_t lt_get_heap_min_free();

/**
 * @brief Get largest block of heap that can be allocated at once.
 */
uint32_t lt_get_heap_max_alloc();

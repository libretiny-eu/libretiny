/* Copyright (c) Kuba Szczodrzyński 2023-03-09. */

#pragma once

#include <libretiny.h>

/**
 * @brief Get total RAM size.
 */
uint32_t lt_ram_get_size();

/**
 * @brief Get total heap size.
 */
uint32_t lt_heap_get_size();

/**
 * @brief Get free heap size.
 */
uint32_t lt_heap_get_free();

/**
 * @brief Get lowest level of free heap memory.
 */
uint32_t lt_heap_get_min_free();

/**
 * @brief Get largest block of heap that can be allocated at once.
 */
uint32_t lt_heap_get_max_alloc();

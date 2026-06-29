/* Copyright (c) Kuba Szczodrzyński 2023-03-09. */

#pragma once

#include <libretiny.h>

/**
 * @brief Get CPU family ID (as lt_cpu_family_t enum member).
 */
lt_cpu_family_t lt_cpu_get_family();

/**
 * @brief Get CPU family name as string.
 */
const char *lt_cpu_get_family_name();

/**
 * @brief Get CPU model ID (as lt_cpu_model_t enum member).
 */
lt_cpu_model_t lt_cpu_get_model();

/**
 * @brief Get CPU model name as string (uppercase).
 */
const char *lt_cpu_get_model_name();

/**
 * @brief Get CPU model name as string (lowercase).
 */
const char *lt_cpu_get_model_code();

/**
 * @brief Get CPU unique ID. This may be based on MAC, eFuse, etc. (family-specific).
 * Note: the number is 24-bit (with the MSB being zero).
 */
uint32_t lt_cpu_get_unique_id();

/**
 * @brief Get CPU ID based on the last three octets of MAC address.
 * Note: the number is 24-bit (with the MSB being zero).
 * The 3rd-to-last octet is least-significant, the last octet is most-significant.
 */
uint32_t lt_cpu_get_mac_id();

/**
 * @brief Get CPU core count.
 */
uint8_t lt_cpu_get_core_count();

/**
 * @brief Get CPU core type name as string.
 */
const char *lt_cpu_get_core_type();

/**
 * @brief Get CPU frequency in Hz.
 */
uint32_t lt_cpu_get_freq();

/**
 * @brief Get CPU frequency in MHz.
 */
uint32_t lt_cpu_get_freq_mhz();

/**
 * @brief Get CPU cycle count.
 */
uint32_t lt_cpu_get_cycle_count();

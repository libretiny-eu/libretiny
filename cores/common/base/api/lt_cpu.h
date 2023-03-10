/* Copyright (c) Kuba Szczodrzyński 2023-03-09. */

#pragma once

#include <libretuya.h>

/**
 * @brief Get CPU family ID (in ChipFamily enumeration).
 */
lt_cpu_family_t lt_get_cpu_family();

/**
 * @brief Get CPU family name as string.
 */
const char *lt_get_cpu_family_name();

/**
 * @brief Get CPU model ID (in ChipType enumeration).
 */
lt_cpu_model_t lt_get_cpu_model();

/**
 * @brief Get CPU model name as string (uppercase).
 */
const char *lt_get_cpu_model_name();

/**
 * @brief Get CPU model name as string (lowercase).
 */
const char *lt_get_cpu_model_code();

/**
 * @brief Get CPU unique ID. This may be based on MAC, eFuse, etc. (family-specific).
 * Note: the number is 24-bit (with the MSB being zero).
 */
uint32_t lt_get_cpu_unique_id();

/**
 * @brief Get CPU ID based on the last three octets of MAC address.
 * Note: the number is 24-bit (with the MSB being zero).
 */
uint32_t lt_get_cpu_mac_id();

/**
 * @brief Get CPU core count.
 */
uint8_t lt_get_cpu_core_count();

/**
 * @brief Get CPU core type name as string.
 */
const char *lt_get_cpu_core_type();

/**
 * @brief Get CPU frequency in Hz.
 */
uint32_t lt_get_cpu_freq();

/**
 * @brief Get CPU frequency in MHz.
 */
uint32_t lt_get_cpu_freq_mhz();

/**
 * @brief Get CPU cycle count.
 */
uint32_t lt_get_cpu_cycle_count();

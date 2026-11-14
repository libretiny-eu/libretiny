#pragma once

#error "Don't include this file directly"

#define LT_HAS_FREERTOS 1
#define LT_HAS_LWIP		1
#define LT_HAS_LWIP2	1
#define LT_HAS_MBEDTLS	1
/* LT_HAS_OTA is supplied by builder/family/realtek-amb1.py: 1 only for the
 * qualification build (custom_amb1_flash_qualification: yes), otherwise 0.
 * It is deliberately absent here because env.LoadDefines() re-appends this
 * file's keys after the family builder and would overwrite the gated value. */
#define LT_HAS_PRINTF	1

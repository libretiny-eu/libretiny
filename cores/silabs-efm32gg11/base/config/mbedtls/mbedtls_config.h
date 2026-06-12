/* Copyright (c) WGM160P-LibreTiny port 2026-06-11.
 *
 * Minimal software-only mbedtls 3.5.0 configuration for WF200 Secure Link on
 * the EFM32GG11. Enables exactly the primitives the FMAC host crypto hooks use
 * (sl_wfx_host_crypto.c): curve25519 ECDH + CTR_DRBG/entropy for the key
 * exchange, HMAC-SHA512/SHA256 for the public-key MAC and KDF, and AES-CCM for
 * per-packet encryption. No PSA, no TLS/x509/net, no Silabs crypto-accelerator
 * plugins — pure software on the Cortex-M4F, which is ample for a one-time ECDH
 * plus small per-frame CCM. Entropy comes from the GG11 TRNG0 via the
 * mbedtls_hardware_poll() in lt_wfx_entropy.c.
 *
 * Selected with -DMBEDTLS_CONFIG_FILE='"mbedtls_config.h"' in the family builder.
 */

#ifndef LT_WFX_MBEDTLS_CONFIG_H
#define LT_WFX_MBEDTLS_CONFIG_H

// ---- Platform -------------------------------------------------------------
// Bare-metal: no OS entropy (Unix/Windows /dev/urandom), use the TRNG via the
// hardware-alt poll instead. mbedtls uses libc calloc/free directly (no
// MBEDTLS_PLATFORM_MEMORY), which newlib-nano provides.
#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_ENTROPY_HARDWARE_ALT

// ---- Big numbers + elliptic curve (curve25519 ECDH) -----------------------
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_ECP_C
#define MBEDTLS_ECDH_C
#define MBEDTLS_ECP_DP_CURVE25519_ENABLED

// ---- Hashes + MAC ---------------------------------------------------------
#define MBEDTLS_MD_C
#define MBEDTLS_SHA256_C
#define MBEDTLS_SHA512_C // also provides SHA-384; needed for HMAC-SHA512

// ---- Symmetric cipher + AEAD (AES-CCM) ------------------------------------
#define MBEDTLS_AES_C
#define MBEDTLS_CIPHER_C // CCM_C prerequisite (mbedtls 3.x check_config)
#define MBEDTLS_CCM_C

// ---- RNG ------------------------------------------------------------------
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_CTR_DRBG_C // AES-CTR DRBG; uses MBEDTLS_AES_C

// ---- Footprint trims ------------------------------------------------------
// No self-tests, error strings, or filesystem in the firmware image.
#define MBEDTLS_AES_FEWER_TABLES

#include "mbedtls/check_config.h"

#endif // LT_WFX_MBEDTLS_CONFIG_H

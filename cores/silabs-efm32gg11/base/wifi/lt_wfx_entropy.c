/* Copyright (c) WGM160P-LibreTiny port 2026-06-11.
 *
 * mbedtls hardware entropy source backed by the EFM32GG11 TRNG0 (a NIST
 * SP800-90B / AIS31 true random number generator with an on-chip conditioner).
 * Used only when Secure Link is enabled: the curve25519 ECDH private key and
 * CTR_DRBG seed for the WF200 key exchange need real entropy, and the GG11 has
 * no other entropy source available to a bare-metal mbedtls build.
 *
 * Software-only mbedtls config: MBEDTLS_NO_PLATFORM_ENTROPY +
 * MBEDTLS_ENTROPY_HARDWARE_ALT route the entropy accumulator to the
 * mbedtls_hardware_poll() defined here. We talk to TRNG0 directly rather than
 * pull in the Silabs PSA / cryptoacc entropy plugin, which is built for
 * SLC-generated projects (device_init, sl_system, psa_crypto) and does not fit
 * a hand-rolled SCons build.
 */

#include "em_cmu.h"
#include "em_device.h"

#include <stddef.h>
#include <stdint.h>

#ifdef SL_CATALOG_WFX_SECURE_LINK_PRESENT

// Health-test alarm flags in TRNG_STATUS. If any fires, the conditioner output
// is suspect; soft-reset and restart the startup tests.
#define LT_TRNG_ALARM_MASK                                                                                         \
	(_TRNG_STATUS_PREIF_MASK | _TRNG_STATUS_ALMIF_MASK | _TRNG_STATUS_APT64IF_MASK | _TRNG_STATUS_APT4096IF_MASK | \
	 TRNG_CONTROL_REPCOUNTIEN) // REPIF shares bit 4 with the repetition-count flag

static int s_trng_started;

static void lt_trng_enable(void) {
	CMU_ClockEnable(cmuClock_TRNG0, true);
	// Soft reset to a known state, then enable with the default conditioned
	// NOISE source (TESTEN=NOISE, CONDBYPASS=NORMAL). The startup health tests
	// run automatically; FIFOLEVEL stays 0 until they pass.
	TRNG0->CONTROL = _TRNG_CONTROL_SOFTRESET_MASK;
	TRNG0->CONTROL = 0;
	TRNG0->CONTROL = TRNG_CONTROL_ENABLE_ENABLED;
}

// mbedtls entropy callback. Fills up to `len` bytes; sets *olen to what we
// produced. Returns 0 on success (mbedtls retries to reach the requested
// MBEDTLS_ENTROPY_MIN_HARDWARE per gather round).
int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen) {
	(void)data;

	if (!s_trng_started) {
		lt_trng_enable();
		s_trng_started = 1;
	}

	size_t produced = 0;
	while (produced < len) {
		// Bounded spin for the FIFO to have a word. The startup tests gate the
		// first words; subsequent refills are fast (the conditioner runs while
		// we are not reading). The timeout degrades to a short read rather than
		// hanging boot if the TRNG ever wedges.
		uint32_t guard = 2000000U;
		while (TRNG0->FIFOLEVEL == 0U && --guard) {
			// On a health-test alarm during startup, restart the tests.
			if (TRNG0->STATUS & LT_TRNG_ALARM_MASK) {
				TRNG0->STATUS = 0; // W1C/clear the alarm flags
				lt_trng_enable();
			}
		}
		if (TRNG0->FIFOLEVEL == 0U) {
			break; // wedged; return what we have
		}

		uint32_t word	 = TRNG0->FIFO;
		size_t remaining = len - produced;
		size_t take		 = remaining < sizeof(word) ? remaining : sizeof(word);
		for (size_t i = 0; i < take; ++i) {
			output[produced++] = (unsigned char)(word >> (8U * i));
		}
	}

	*olen = produced;
	return 0;
}

#endif // SL_CATALOG_WFX_SECURE_LINK_PRESENT

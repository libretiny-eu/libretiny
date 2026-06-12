/* Copyright (c) WGM160P-LibreTiny port 2026-06-12.
 *
 * WF200 Secure Link host crypto hooks — an independent mbedtls implementation
 * of the seven sl_wfx_host_* SecureLink callbacks declared (Apache-2.0) in the
 * wfx-fullMAC driver's sl_wfx_host_api.h. Nothing here is copied from the Gecko
 * SDK example host code (which ships under the proprietary Silicon Labs MSLA);
 * it is written from the published hook contracts, the public message layout in
 * sl_wfx_general_api.h, and the SecureLink wire protocol enforced by the WF200
 * silicon. The crypto choices below are fixed by that protocol (the device
 * computes the matching values its own way), so they are interoperability
 * requirements, not design decisions:
 *
 *   - Key agreement: X25519 (Curve25519 ECDH). Public keys travel as 32 raw
 *     little-endian bytes; mbedtls works big-endian, so each key is reversed at
 *     the wire boundary.
 *   - Public-key authentication: HMAC-SHA512 keyed with the 32-byte part MAC
 *     key, producing the 64-byte host_pub_key_mac / ncp_pub_key_mac fields.
 *   - Session key: SHA-256 of the (little-endian) shared secret, first 16 bytes.
 *   - Record cipher: AES-128-CCM, 12-byte nonce built from the {hp, rx, tx}
 *     packet counters, 16-byte tag, no associated data.
 *
 * The kernel objects (rx mutex, renegotiation task handle) live in
 * sl_wfx_secure_link_task.c. mbedtls entropy is fed by the GG11 TRNG via
 * mbedtls_hardware_poll() (lt_wfx_entropy.c).
 */

#include "sl_wfx.h"

#ifdef SL_WFX_USE_SECURE_LINK

#if !SL_WFX_SLK_CURVE25519
#error "This WF200 host port implements the Curve25519 SecureLink mode only."
#endif

#include <string.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

// mbedtls 3.x marks the ecp_point / ecp_group coordinate fields private; opt in
// so the X/Z montgomery coordinates can be marshalled to and from the wire.
#define MBEDTLS_ALLOW_PRIVATE_ACCESS
#include "mbedtls/ccm.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/ecp.h"
#include "mbedtls/entropy.h"
#include "mbedtls/md.h"
#include "mbedtls/sha256.h"

// The provisioned 32-byte MAC key sits in the GG11 device-information page (FCCC
// base 0x0FE08000 + 0x3D0) — read-only flash, present on parts burned for
// ENFORCED SecureLink. An unwritten slot reads all-0xFF.
#define LT_WFX_MAC_KEY_ADDR ((const uint8_t *)(0x0FE08000UL + 0x3D0UL))

// Owned by sl_wfx_secure_link_task.c.
extern SemaphoreHandle_t wfx_securelink_rx_mutex;
extern TaskHandle_t wfx_securelink_task_handle;

// X25519 agreement state. The ephemeral private scalar is generated in
// compute_pub_key() and consumed in verify_pub_key(), so it must persist across
// the two calls; all of it is released in free_crypto_context().
static mbedtls_entropy_context lt_entropy;
static mbedtls_ctr_drbg_context lt_drbg;
static mbedtls_ecp_group lt_curve;
static mbedtls_mpi lt_priv;		 // host ephemeral private scalar
static mbedtls_ecp_point lt_pub; // host ephemeral public point
static int lt_agreement_ready;

// In-place byte reversal — Curve25519 keys are little-endian on the wire,
// big-endian in mbedtls mpis.
static void lt_reverse(uint8_t *p, size_t len) {
	for (size_t i = 0, j = len - 1; i < j; i++, j--) {
		uint8_t t = p[i];
		p[i]	  = p[j];
		p[j]	  = t;
	}
}

sl_status_t sl_wfx_host_get_secure_link_mac_key(uint8_t *sl_mac_key) {
	memcpy(sl_mac_key, LT_WFX_MAC_KEY_ADDR, SL_WFX_SECURE_LINK_MAC_KEY_LENGTH);
	// A key burned into the DI page has at least one non-0xFF byte; an all-0xFF
	// read means the part was never provisioned.
	for (uint32_t i = 0; i < SL_WFX_SECURE_LINK_MAC_KEY_LENGTH; i++) {
		if (sl_mac_key[i] != 0xFF)
			return SL_STATUS_OK;
	}
	return SL_STATUS_WIFI_SECURE_LINK_MAC_KEY_ERROR;
}

sl_status_t sl_wfx_host_compute_pub_key(
	sl_wfx_securelink_exchange_pub_keys_req_body_t *request,
	const uint8_t *sl_mac_key
) {
	static const char drbg_pers[] = "lt-wfx-securelink";

	mbedtls_entropy_init(&lt_entropy);
	mbedtls_ctr_drbg_init(&lt_drbg);
	mbedtls_ecp_group_init(&lt_curve);
	mbedtls_mpi_init(&lt_priv);
	mbedtls_ecp_point_init(&lt_pub);
	lt_agreement_ready = 0;

	if (mbedtls_ctr_drbg_seed(
			&lt_drbg,
			mbedtls_entropy_func,
			&lt_entropy,
			(const unsigned char *)drbg_pers,
			sizeof(drbg_pers)
		) != 0)
		return SL_STATUS_WIFI_SECURE_LINK_EXCHANGE_FAILED;

	if (mbedtls_ecp_group_load(&lt_curve, MBEDTLS_ECP_DP_CURVE25519) != 0)
		return SL_STATUS_WIFI_SECURE_LINK_EXCHANGE_FAILED;

	// Ephemeral X25519 key pair; the u-coordinate (point.X) is the public key.
	if (mbedtls_ecdh_gen_public(&lt_curve, &lt_priv, &lt_pub, mbedtls_ctr_drbg_random, &lt_drbg) != 0)
		return SL_STATUS_WIFI_SECURE_LINK_EXCHANGE_FAILED;
	if (mbedtls_mpi_write_binary(&lt_pub.X, request->host_pub_key, SL_WFX_HOST_PUB_KEY_SIZE) != 0)
		return SL_STATUS_WIFI_SECURE_LINK_EXCHANGE_FAILED;
	lt_reverse(request->host_pub_key, SL_WFX_HOST_PUB_KEY_SIZE);
	lt_agreement_ready = 1;

	// Authenticate the public key for the device: HMAC-SHA512 under the MAC key.
	if (mbedtls_md_hmac(
			mbedtls_md_info_from_type(MBEDTLS_MD_SHA512),
			sl_mac_key,
			SL_WFX_SECURE_LINK_MAC_KEY_LENGTH,
			request->host_pub_key,
			SL_WFX_HOST_PUB_KEY_SIZE,
			request->host_pub_key_mac
		) != 0)
		return SL_STATUS_WIFI_SECURE_LINK_EXCHANGE_FAILED;

	return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_verify_pub_key(
	sl_wfx_securelink_exchange_pub_keys_ind_t *response_packet,
	const uint8_t *sl_mac_key,
	uint8_t *sl_host_pub_key
) {
	SL_WFX_UNUSED_PARAMETER(sl_host_pub_key);

	uint8_t expected_mac[SL_WFX_NCP_PUB_KEY_MAC_SIZE];
	uint8_t secret[SL_WFX_HOST_PUB_KEY_SIZE];
	uint8_t digest[32];
	mbedtls_ecp_point peer;
	mbedtls_mpi shared;
	sl_status_t status = SL_STATUS_WIFI_SECURE_LINK_EXCHANGE_FAILED;

	if (!lt_agreement_ready)
		return SL_STATUS_WIFI_SECURE_LINK_EXCHANGE_FAILED;
	if (xSemaphoreTake(wfx_securelink_rx_mutex, portMAX_DELAY) != pdTRUE)
		return SL_STATUS_WIFI_SECURE_LINK_EXCHANGE_FAILED;

	mbedtls_ecp_point_init(&peer);
	mbedtls_mpi_init(&shared);

	// Authenticate the device public key before trusting it.
	if (mbedtls_md_hmac(
			mbedtls_md_info_from_type(MBEDTLS_MD_SHA512),
			sl_mac_key,
			SL_WFX_SECURE_LINK_MAC_KEY_LENGTH,
			response_packet->body.ncp_pub_key,
			SL_WFX_NCP_PUB_KEY_SIZE,
			expected_mac
		) != 0)
		goto out;
	if (memcmp(expected_mac, response_packet->body.ncp_pub_key_mac, SL_WFX_NCP_PUB_KEY_MAC_SIZE) != 0)
		goto out;

	// Load the device u-coordinate (Z = 1 for the affine montgomery point).
	lt_reverse(response_packet->body.ncp_pub_key, SL_WFX_NCP_PUB_KEY_SIZE);
	if (mbedtls_mpi_read_binary(&peer.X, response_packet->body.ncp_pub_key, SL_WFX_NCP_PUB_KEY_SIZE) != 0)
		goto out;
	if (mbedtls_mpi_lset(&peer.Z, 1) != 0)
		goto out;

	// X25519(host_priv, device_pub) -> shared secret.
	if (mbedtls_ecdh_compute_shared(&lt_curve, &shared, &peer, &lt_priv, mbedtls_ctr_drbg_random, &lt_drbg) != 0)
		goto out;
	if (mbedtls_mpi_write_binary(&shared, secret, sizeof(secret)) != 0)
		goto out;
	lt_reverse(secret, sizeof(secret));

	// Session key = first 16 bytes of SHA-256(shared secret).
	if (mbedtls_sha256(secret, sizeof(secret), digest, 0) != 0)
		goto out;
	memcpy(sl_wfx_context->secure_link_session_key, digest, SL_WFX_SECURE_LINK_SESSION_KEY_LENGTH);
	sl_wfx_context->secure_link_nonce.hp_packet_count = 0;
	sl_wfx_context->secure_link_nonce.rx_packet_count = 0;
	sl_wfx_context->secure_link_nonce.tx_packet_count = 0;
	status											  = SL_STATUS_OK;

out:
	mbedtls_mpi_free(&shared);
	mbedtls_ecp_point_free(&peer);
	xSemaphoreGive(wfx_securelink_rx_mutex);
	return status;
}

sl_status_t sl_wfx_host_free_crypto_context(void) {
	mbedtls_ecp_point_free(&lt_pub);
	mbedtls_mpi_free(&lt_priv);
	mbedtls_ecp_group_free(&lt_curve);
	mbedtls_ctr_drbg_free(&lt_drbg);
	mbedtls_entropy_free(&lt_entropy);
	lt_agreement_ready = 0;
	return SL_STATUS_OK;
}

// length excludes the SecureLink header and the trailing CCM tag.
sl_status_t sl_wfx_host_decode_secure_link_data(uint8_t *buffer, uint32_t length, uint8_t *session_key) {
	mbedtls_ccm_context ccm;
	sl_status_t status = SL_STATUS_SECURITY_DECRYPT_ERROR;

	if (xSemaphoreTake(wfx_securelink_rx_mutex, portMAX_DELAY) != pdTRUE)
		return SL_STATUS_FAIL;

	// Receive nonce uses only the rx counter; hp and tx stay zero.
	sl_wfx_nonce_t nonce  = {0, 0, 0};
	nonce.rx_packet_count = sl_wfx_context->secure_link_nonce.rx_packet_count;

	mbedtls_ccm_init(&ccm);
	if (mbedtls_ccm_setkey(&ccm, MBEDTLS_CIPHER_ID_AES, session_key, SL_WFX_SECURE_LINK_SESSION_KEY_BIT_COUNT) == 0 &&
		mbedtls_ccm_auth_decrypt(
			&ccm,
			length,
			(uint8_t *)&nonce,
			SL_WFX_SECURE_LINK_NONCE_SIZE_BYTES,
			NULL,
			0,
			buffer,
			buffer,
			buffer + length,
			SL_WFX_SECURE_LINK_CCM_TAG_SIZE
		) == 0) {
		status = SL_STATUS_OK;
	}

	mbedtls_ccm_free(&ccm);
	xSemaphoreGive(wfx_securelink_rx_mutex);
	return status;
}

// data_length excludes the SecureLink header and the trailing CCM tag; the
// driver supplies the tx nonce. Plaintext begins at the message id field.
sl_status_t sl_wfx_host_encode_secure_link_data(
	sl_wfx_generic_message_t *buffer,
	uint32_t data_length,
	uint8_t *session_key,
	uint8_t *nonce
) {
	mbedtls_ccm_context ccm;
	sl_status_t status = SL_STATUS_FAIL;
	uint8_t *plaintext = (uint8_t *)&buffer->header.id;

	mbedtls_ccm_init(&ccm);
	if (mbedtls_ccm_setkey(&ccm, MBEDTLS_CIPHER_ID_AES, session_key, SL_WFX_SECURE_LINK_SESSION_KEY_BIT_COUNT) == 0 &&
		mbedtls_ccm_encrypt_and_tag(
			&ccm,
			data_length,
			nonce,
			SL_WFX_SECURE_LINK_NONCE_SIZE_BYTES,
			NULL,
			0,
			plaintext,
			plaintext,
			plaintext + data_length,
			SL_WFX_SECURE_LINK_CCM_TAG_SIZE
		) == 0) {
		status = SL_STATUS_OK;
	}

	mbedtls_ccm_free(&ccm);
	return status;
}

sl_status_t sl_wfx_host_schedule_secure_link_renegotiation(void) {
	// Hand off to the SecureLink task, which calls
	// sl_wfx_secure_link_renegotiate_session_key() outside this bus-RX context.
	xTaskNotifyGive(wfx_securelink_task_handle);
	return SL_STATUS_OK;
}

#endif // SL_WFX_USE_SECURE_LINK

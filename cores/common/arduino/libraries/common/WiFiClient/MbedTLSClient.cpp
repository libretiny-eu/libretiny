/* Copyright (c) Kuba Szczodrzyński 2022-04-30. */

#if LT_ARD_HAS_WIFI && LT_HAS_MBEDTLS

#include "MbedTLSClient.h"

#include <WiFi.h>

extern "C" {

#include <mbedtls/debug.h>
#include <mbedtls/net.h>
#include <mbedtls/pk.h>
#include <mbedtls/platform.h>
#include <mbedtls/sha256.h>
#include <mbedtls/ssl.h>

} // extern "C"

#define _clientKeyC ((mbedtls_pk_context *)_clientKey)

MbedTLSClient::MbedTLSClient() : WiFiClient() {
	init(); // ensure the context is zero filled
}

MbedTLSClient::MbedTLSClient(int sock) : WiFiClient(sock) {
	init(); // ensure the context is zero filled
}

MbedTLSClient::~MbedTLSClient() {
	LT_VM(CLIENT, "~MbedTLSClient()");
	stop();
}

void MbedTLSClient::stop() {
	if (!_sslCtx)
		return;
	LT_VM(SSL, "Stopping SSL");

	if (_sslCfg->ca_chain) {
		mbedtls_x509_crt_free(_caCert);
	}
	if (_sslCfg->key_cert) {
		mbedtls_x509_crt_free(_clientCert);
		mbedtls_pk_free(_clientKeyC);
	}
	mbedtls_ssl_free(_sslCtx);
	mbedtls_ssl_config_free(_sslCfg);

	free(_sslCtx);
	free(_sslCfg);
	free(_caCert);
	free(_clientCert);
	free(_clientKey);
	_sslCtx = NULL;

	LT_HEAP_I();
}

void MbedTLSClient::init() {
	if (!_sslCtx) {
		_sslCtx		= (mbedtls_ssl_context *)malloc(sizeof(mbedtls_ssl_context));
		_sslCfg		= (mbedtls_ssl_config *)malloc(sizeof(mbedtls_ssl_config));
		_caCert		= (mbedtls_x509_crt *)malloc(sizeof(mbedtls_x509_crt));
		_clientCert = (mbedtls_x509_crt *)malloc(sizeof(mbedtls_x509_crt));
		_clientKey	= (mbedtls_pk_context *)malloc(sizeof(mbedtls_pk_context));
	}
	// Realtek AmbZ: init platform here to ensure HW crypto is initialized in ssl_init
	mbedtls_platform_set_calloc_free(calloc, free);
	mbedtls_ssl_init(_sslCtx);
	mbedtls_ssl_config_init(_sslCfg);
}

int MbedTLSClient::connect(IPAddress ip, uint16_t port, int32_t timeout) {
	return connect(ipToString(ip).c_str(), port, timeout);
}

int MbedTLSClient::connect(const char *host, uint16_t port, int32_t timeout) {
	if (_pskIdentStr && _pskStr)
		return connect(host, port, timeout, NULL, NULL, NULL, _pskIdentStr, _pskStr) == 0;
	return connect(host, port, timeout, _caCertStr, _clientCertStr, _clientKeyStr, NULL, NULL) == 0;
}

int MbedTLSClient::connect(
	IPAddress ip, uint16_t port, const char *rootCABuf, const char *clientCert, const char *clientKey
) {
	return connect(ipToString(ip).c_str(), port, 0, rootCABuf, clientCert, clientKey, NULL, NULL) == 0;
}

int MbedTLSClient::connect(
	const char *host, uint16_t port, const char *rootCABuf, const char *clientCert, const char *clientKey
) {
	return connect(host, port, 0, rootCABuf, clientCert, clientKey, NULL, NULL) == 0;
}

int MbedTLSClient::connect(IPAddress ip, uint16_t port, const char *pskIdent, const char *psk) {
	return connect(ipToString(ip).c_str(), port, 0, NULL, NULL, NULL, pskIdent, psk) == 0;
}

int MbedTLSClient::connect(const char *host, uint16_t port, const char *pskIdent, const char *psk) {
	return connect(host, port, 0, NULL, NULL, NULL, pskIdent, psk) == 0;
}

static int ssl_random(void *data, unsigned char *output, size_t len) {
	lt_rand_bytes((uint8_t *)output, len);
	return 0;
}

void debug_cb(void *ctx, int level, const char *file, int line, const char *str) {
	// do not print the trailing \n
	uint16_t len = strlen(str);
	char *msg	 = (char *)str;
	msg[len - 1] = '\0';
	LT_IM(SSL, "%04d: |%d| %s", line, level, msg);
}

int MbedTLSClient::connect(
	const char *host,
	uint16_t port,
	int32_t timeout,
	const char *rootCABuf,
	const char *clientCert,
	const char *clientKey,
	const char *pskIdent,
	const char *psk
) {
	LT_HEAP_I();

	if (!rootCABuf && !pskIdent && !psk && !_insecure && !_useRootCA)
		return -1;

	if (timeout <= 0)
		timeout = _timeout; // use default when -1 passed as timeout

	IPAddress addr = WiFi.hostByName(host);
	if (!(uint32_t)addr)
		return -1;

	int ret = WiFiClient::connect(addr, port, timeout);
	if (ret < 0) {
		LT_EM(SSL, "SSL socket failed");
		return ret;
	}

	char *uid = "lt-ssl"; // TODO

	LT_VM(SSL, "Init SSL");
	init();
	LT_HEAP_I();

	// mbedtls_debug_set_threshold(4);
	// mbedtls_ssl_conf_dbg(&_sslCfg, debug_cb, NULL);

	ret = mbedtls_ssl_config_defaults(
		_sslCfg,
		MBEDTLS_SSL_IS_CLIENT,
		MBEDTLS_SSL_TRANSPORT_STREAM,
		MBEDTLS_SSL_PRESET_DEFAULT
	);
	LT_RET_NZ(ret);

#ifdef MBEDTLS_SSL_ALPN
	if (_alpnProtocols) {
		ret = mbedtls_ssl_conf_alpn_protocols(&_sslCfg, _alpnProtocols);
		LT_RET_NZ(ret);
	}
#endif

	if (_insecure) {
		mbedtls_ssl_conf_authmode(_sslCfg, MBEDTLS_SSL_VERIFY_NONE);
	} else if (rootCABuf) {
		mbedtls_x509_crt_init(_caCert);
		mbedtls_ssl_conf_authmode(_sslCfg, MBEDTLS_SSL_VERIFY_REQUIRED);
		ret = mbedtls_x509_crt_parse(_caCert, (const unsigned char *)rootCABuf, strlen(rootCABuf) + 1);
		mbedtls_ssl_conf_ca_chain(_sslCfg, _caCert, NULL);
		if (ret < 0) {
			mbedtls_x509_crt_free(_caCert);
			LT_RET(ret);
		}
	} else if (_useRootCA) {
		return -1; // not implemented
	} else if (pskIdent && psk) {
#ifdef MBEDTLS_KEY_EXCHANGE__SOME__PSK_ENABLED
		uint16_t len = strlen(psk);
		if ((len & 1) != 0 || len > 2 * MBEDTLS_PSK_MAX_LEN) {
			LT_EM(SSL, "PSK length invalid");
			return -1;
		}
		unsigned char pskBin[MBEDTLS_PSK_MAX_LEN] = {};
		for (uint8_t i = 0; i < len; i++) {
			uint8_t c = psk[i];
			c |= 0b00100000; // make lowercase
			c -= '0' * (c >= '0' && c <= '9');
			c -= ('a' - 10) * (c >= 'a' && c <= 'z');
			if (c > 0xf)
				return -1;
			pskBin[i / 2] |= c << (4 * ((i & 1) ^ 1));
		}
		ret = mbedtls_ssl_conf_psk(_sslCfg, pskBin, len / 2, (const unsigned char *)pskIdent, strlen(pskIdent));
		LT_RET_NZ(ret);
#else
		return -1;
#endif
	} else {
		return -1;
	}

	if (!_insecure && clientCert && clientKey) {
		mbedtls_x509_crt_init(_clientCert);
		mbedtls_pk_init(_clientKeyC);
		LT_VM(SSL, "Loading client cert");
		ret = mbedtls_x509_crt_parse(_clientCert, (const unsigned char *)clientCert, strlen(clientCert) + 1);
		if (ret < 0) {
			mbedtls_x509_crt_free(_clientCert);
			LT_RET(ret);
		}
		LT_VM(SSL, "Loading private key");
		ret = mbedtls_pk_parse_key(_clientKeyC, (const unsigned char *)clientKey, strlen(clientKey) + 1, NULL, 0);
		if (ret < 0) {
			mbedtls_x509_crt_free(_clientCert);
			LT_RET(ret);
		}
		mbedtls_ssl_conf_own_cert(_sslCfg, _clientCert, _clientKeyC);
	}

	LT_VM(SSL, "Setting TLS hostname");
	ret = mbedtls_ssl_set_hostname(_sslCtx, host);
	LT_RET_NZ(ret);

	mbedtls_ssl_conf_rng(_sslCfg, ssl_random, NULL);
	ret = mbedtls_ssl_setup(_sslCtx, _sslCfg);
	LT_RET_NZ(ret);

	_sockTls = fd();
	mbedtls_ssl_set_bio(_sslCtx, &_sockTls, mbedtls_net_send, mbedtls_net_recv, NULL);
	mbedtls_net_set_nonblock((mbedtls_net_context *)&_sockTls);

	LT_HEAP_I();

	LT_VM(SSL, "SSL handshake");
	if (_handshakeTimeout == 0)
		_handshakeTimeout = timeout;
	unsigned long start = millis();
	while (ret = mbedtls_ssl_handshake(_sslCtx)) {
		if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
			LT_RET(ret);
		}
		if ((millis() - start) > _handshakeTimeout) {
			LT_EM(SSL, "SSL handshake timeout");
			return -1;
		}
		delay(2);
	}

	LT_HEAP_I();

	if (clientCert && clientKey) {
		LT_DM(
			SSL,
			"Protocol %s, ciphersuite %s",
			mbedtls_ssl_get_version(_sslCtx),
			mbedtls_ssl_get_ciphersuite(_sslCtx)
		);
		ret = mbedtls_ssl_get_record_expansion(_sslCtx);
		if (ret >= 0)
			LT_DM(SSL, "Record expansion: %d", ret);
		else {
			LT_WM(SSL, "Record expansion unknown");
		}
	}

	LT_VM(SSL, "Verifying certificate");
	ret = mbedtls_ssl_get_verify_result(_sslCtx);
	if (ret) {
		char buf[512];
		memset(buf, 0, sizeof(buf));
		mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", ret);
		LT_EM(SSL, "Failed to verify peer certificate! Verification info: %s", buf);
		return ret;
	}

	if (rootCABuf)
		mbedtls_x509_crt_free(_caCert);
	if (clientCert)
		mbedtls_x509_crt_free(_clientCert);
	if (clientKey != NULL)
		mbedtls_pk_free(_clientKeyC);
	return 0; // OK
}

size_t MbedTLSClient::write(const uint8_t *buf, size_t size) {
	int ret = -1;
	while ((ret = mbedtls_ssl_write(_sslCtx, buf, size)) <= 0) {
		if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret < 0) {
			LT_RET(ret);
		}
		delay(2);
	}
	return ret;
}

int MbedTLSClient::available() {
	bool peeked = _peeked >= 0;
	if (!connected())
		return peeked;

	int ret = mbedtls_ssl_read(_sslCtx, NULL, 0);
	if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret < 0) {
		stop();
		return peeked ? peeked : ret;
	}
	return mbedtls_ssl_get_bytes_avail(_sslCtx) + peeked;
}

int MbedTLSClient::read(uint8_t *buf, size_t size) {
	bool peeked = false;
	int toRead	= available();
	if ((!buf && size) || toRead <= 0)
		return -1;
	if (!size)
		return 0;
	if (_peeked >= 0) {
		buf[0]	= _peeked;
		_peeked = -1;
		size--;
		toRead--;
		if (!size || !toRead)
			return 1;
		buf++;
		peeked = true;
	}

	int ret = mbedtls_ssl_read(_sslCtx, buf, size);
	if (ret < 0) {
		stop();
		return peeked ? peeked : ret;
	}
	return ret + peeked;
}

int MbedTLSClient::peek() {
	if (_peeked >= 0)
		return _peeked;
	_peeked = timedRead();
	return _peeked;
}

void MbedTLSClient::flush() {}

int MbedTLSClient::lastError(char *buf, const size_t size) {
	return 0; // TODO (?)
}

void MbedTLSClient::setInsecure() {
	_caCertStr	   = NULL;
	_clientCertStr = NULL;
	_clientKeyStr  = NULL;
	_pskIdentStr   = NULL;
	_pskStr		   = NULL;
	_insecure	   = true;
}

// TODO only allocate _caCert, _clientCert and _clientKey when one
// of the following functions is used

void MbedTLSClient::setPreSharedKey(const char *pskIdent, const char *psk) {
	_pskIdentStr = pskIdent;
	_pskStr		 = psk;
}

void MbedTLSClient::setCACert(const char *rootCA) {
	_caCertStr = rootCA;
}

void MbedTLSClient::setCertificate(const char *clientCA) {
	_clientCertStr = clientCA;
}

void MbedTLSClient::setPrivateKey(const char *privateKey) {
	_clientKeyStr = privateKey;
}

char *streamToStr(Stream &stream, size_t size) {
	char *buf = (char *)malloc(size + 1);
	if (!buf)
		return NULL;
	if (size != stream.readBytes(buf, size)) {
		free(buf);
		return NULL;
	}
	buf[size] = '\0';
	return buf;
}

bool MbedTLSClient::loadCACert(Stream &stream, size_t size) {
	char *str = streamToStr(stream, size);
	if (str) {
		_caCertStr = str;
		return true;
	}
	return false;
}

bool MbedTLSClient::loadCertificate(Stream &stream, size_t size) {
	char *str = streamToStr(stream, size);
	if (str) {
		_clientCertStr = str;
		return true;
	}
	return false;
}

bool MbedTLSClient::loadPrivateKey(Stream &stream, size_t size) {
	char *str = streamToStr(stream, size);
	if (str) {
		_clientKeyStr = str;
		return true;
	}
	return false;
}

bool MbedTLSClient::verify(const char *fingerprint, const char *domainName) {
	uint8_t fpLocal[32] = {};
	uint16_t len		= strlen(fingerprint);
	uint8_t byte		= 0;
	for (uint8_t i = 0; i < len; i++) {
		uint8_t c = fingerprint[i];
		while ((c == ' ' || c == ':') && i < len) {
			c = fingerprint[++i];
		}
		c |= 0b00100000; // make lowercase
		c -= '0' * (c >= '0' && c <= '9');
		c -= ('a' - 10) * (c >= 'a' && c <= 'z');
		if (c > 0xf)
			return -1;
		fpLocal[byte / 2] |= c << (4 * ((byte & 1) ^ 1));
		byte++;
		if (byte >= 64)
			break;
	}

	uint8_t fpRemote[32];
	if (!getFingerprintSHA256(fpRemote))
		return false;

	if (memcmp(fpLocal, fpRemote, 32)) {
		LT_DM(SSL, "Fingerprints don't match");
		return false;
	}

	if (!domainName)
		return true;
	// TODO domain name verification
	return true;
}

void MbedTLSClient::setHandshakeTimeout(unsigned long handshakeTimeout) {
	_handshakeTimeout = handshakeTimeout * 1000;
}

void MbedTLSClient::setAlpnProtocols(const char **alpnProtocols) {
	_alpnProtocols = alpnProtocols;
}

bool MbedTLSClient::getFingerprintSHA256(uint8_t result[32]) {
	const mbedtls_x509_crt *cert = mbedtls_ssl_get_peer_cert(_sslCtx);
	if (!cert) {
		LT_EM(SSL, "Failed to get peer certificate");
		return false;
	}
	mbedtls_sha256_context shaCtx;
	mbedtls_sha256_init(&shaCtx);
	mbedtls_sha256_starts(&shaCtx, false);
	mbedtls_sha256_update(&shaCtx, cert->raw.p, cert->raw.len);
	mbedtls_sha256_finish(&shaCtx, result);
	return true;
}

#endif // LT_ARD_HAS_WIFI && LT_HAS_MBEDTLS

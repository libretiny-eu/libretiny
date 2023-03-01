/* Copyright (c) Kuba Szczodrzyński 2022-04-30. */

#pragma once

#if LT_ARD_HAS_WIFI && LT_HAS_MBEDTLS

#include "WiFiClientSecure.h"

struct mbedtls_ssl_context;
struct mbedtls_ssl_config;
struct mbedtls_x509_crt;

class MbedTLSClient : public WiFiClient, public IWiFiClientSecure {
  private:
	mbedtls_ssl_context *_sslCtx = NULL;
	mbedtls_ssl_config *_sslCfg;
	mbedtls_x509_crt *_caCert;
	mbedtls_x509_crt *_clientCert;
	void *_clientKey;
	uint32_t _handshakeTimeout = 0;

	void init();
	int _sockTls	= -1;
	bool _insecure	= false;
	bool _useRootCA = false;
	int _peeked		= -1;

	const char *_caCertStr;
	const char *_clientCertStr;
	const char *_clientKeyStr;
	const char *_pskIdentStr;
	const char *_pskStr;
	const char **_alpnProtocols;

	int connect(
		const char *host,
		uint16_t port,
		int32_t timeout,
		const char *rootCABuf,
		const char *clientCert,
		const char *clientKey,
		const char *pskIdent,
		const char *psk
	);

  public:
	MbedTLSClient();
	MbedTLSClient(int sock);
	~MbedTLSClient();

	int connect(IPAddress ip, uint16_t port, int32_t timeout);
	int connect(const char *host, uint16_t port, int32_t timeout);

	int connect(IPAddress ip, uint16_t port, const char *rootCABuf, const char *clientCert, const char *clientKey);
	int connect(const char *host, uint16_t port, const char *rootCABuf, const char *clientCert, const char *clientKey);
	int connect(IPAddress ip, uint16_t port, const char *pskIdent, const char *psk);
	int connect(const char *host, uint16_t port, const char *pskIdent, const char *psk);

	size_t write(const uint8_t *buf, size_t size);

	int available();

	int read(uint8_t *buf, size_t size);
	int peek();
	void flush();
	void stop();

	int lastError(char *buf, const size_t size);
	void setInsecure(); // Don't validate the chain, just accept whatever is given. VERY INSECURE!
	void setPreSharedKey(const char *pskIdent, const char *psk); // psk in hex
	void setCACert(const char *rootCA);
	void setCertificate(const char *clientCA);
	void setPrivateKey(const char *privateKey);
	bool loadCACert(Stream &stream, size_t size);
	bool loadCertificate(Stream &stream, size_t size);
	bool loadPrivateKey(Stream &stream, size_t size);
	bool verify(const char *fingerprint, const char *domainName);
	void setHandshakeTimeout(unsigned long handshakeTimeout);
	void setAlpnProtocols(const char **alpnProtocols);
	bool getFingerprintSHA256(uint8_t result[32]);

	using WiFiClient::connect;
	using WiFiClient::read;
};

typedef MbedTLSClient WiFiClientSecure;

#endif

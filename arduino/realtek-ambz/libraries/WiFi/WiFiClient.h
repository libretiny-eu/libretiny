/* Copyright (c) Kuba Szczodrzyński 2022-04-26. */

#pragma once

#include <LwIPRxBuffer.h>
#include <api/WiFiClient.h>
#include <memory>

class SocketHandle;

class WiFiClient : public IWiFiClient {
  private:
	bool _connected;
	std::shared_ptr<SocketHandle> _sock;
	std::shared_ptr<LwIPRxBuffer> _rxBuffer;

  public:
	WiFiClient();
	WiFiClient(int sock);
	~WiFiClient();

	int connect(IPAddress ip, uint16_t port);
	int connect(const char *host, uint16_t port);
	int connect(IPAddress ip, uint16_t port, int32_t timeout);
	int connect(const char *host, uint16_t port, int32_t timeout);

	size_t write(uint8_t data);
	size_t write(const uint8_t *buf, size_t size);
	size_t write(Stream &stream);

	int available();
	int fd() const;
	int socket();
	int setTimeout(uint32_t seconds);

	int read();
	int read(uint8_t *buf, size_t size);
	int peek();
	void flush();
	void stop();
	uint8_t connected();

	WiFiClient &operator=(const WiFiClient &other);

	IPAddress remoteIP() const;
	IPAddress remoteIP(int sock) const;
	uint16_t remotePort() const;
	uint16_t remotePort(int sock) const;
	IPAddress localIP() const;
	IPAddress localIP(int sock) const;
	uint16_t localPort() const;
	uint16_t localPort(int sock) const;

	using Print::write;
};

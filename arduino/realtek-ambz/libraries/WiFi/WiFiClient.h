#pragma once

#include <api/LwIPRxBuffer.h>
#include <api/WiFiClient.h>

class WiFiClient : public IWiFiClient {
  private:
	int _sock;
	bool _connected;
	LwIPRxBuffer *_rxBuffer;

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

	WiFiClient &operator=(const IWiFiClient &other);

	bool operator==(const IWiFiClient &other) const;

	IPAddress remoteIP() const;
	IPAddress remoteIP(int sock) const;
	uint16_t remotePort() const;
	uint16_t remotePort(int sock) const;
	IPAddress localIP() const;
	IPAddress localIP(int sock) const;
	uint16_t localPort() const;
	uint16_t localPort(int sock) const;
};

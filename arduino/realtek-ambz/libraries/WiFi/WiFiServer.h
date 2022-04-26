#pragma once

#include <api/WiFiServer.h>

#include "WiFiClient.h"

class WiFiServer : public IWiFiServer<WiFiClient> {
  private:
	int _sock;
	int _sockAccepted;
	uint16_t _port;
	uint8_t _maxClients;
	bool _active;
	bool _noDelay = false;

  public:
	WiFiServer(uint16_t port = 80, uint8_t maxClients = 4);

	operator bool();

	bool begin(uint16_t port = 0, bool reuseAddr = true);
	void end();
	WiFiClient accept();

	size_t write(const uint8_t *buffer, size_t size) {
		return 0;
	}

	void stopAll() {}

	int setTimeout(uint32_t seconds);
	void setNoDelay(bool noDelay);
	bool getNoDelay();
	bool hasClient();
};

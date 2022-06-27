/* Copyright (c) Kuba Szczodrzyński 2022-04-26. */

#pragma once

#include <api/WiFi/WiFi.h>
#include <api/WiFiServer.h>

#include <WiFiClient.h>

class LwIPServer : public IWiFiServer<WiFiClient> {
  private:
	int _sock;
	int _sockAccepted;
	uint32_t _addr;
	uint16_t _port;
	uint8_t _maxClients;
	bool _active;
	bool _noDelay = false;

  private:
	LwIPServer(uint32_t addr, uint16_t port = 80, uint8_t maxClients = 4);

  public:
	LwIPServer(uint16_t port = 80, uint8_t maxClients = 4) : LwIPServer((uint32_t)0, port, maxClients) {}

	LwIPServer(int port = 80, uint8_t maxClients = 4) : LwIPServer((uint32_t)0, port, maxClients) {}

	LwIPServer(const IPAddress &addr, uint16_t port = 80, uint8_t maxClients = 4)
		: LwIPServer((uint32_t)addr, port, maxClients) {}

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

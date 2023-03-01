/* Copyright (c) Kuba Szczodrzyński 2022-09-10. */

#pragma once

#if LT_ARD_HAS_WIFI && LT_HAS_LWIP

#include "WiFiUdp.h"

#include <cbuf.h>

class LwIPUDP : public IWiFiUDP {
  private:
	int udp_server;
	IPAddress multicast_ip;
	IPAddress remote_ip;
	uint16_t server_port;
	uint16_t remote_port;
	char *tx_buffer;
	size_t tx_buffer_len;
	cbuf *rx_buffer;

  public:
	LwIPUDP();
	~LwIPUDP();
	uint8_t begin(IPAddress ip, uint16_t port);
	uint8_t begin(uint16_t port);
	uint8_t beginMulticast(IPAddress ip, uint16_t port);
	void stop();
	int beginMulticastPacket();
	int beginPacket();
	int beginPacket(IPAddress ip, uint16_t port);
	int beginPacket(const char *host, uint16_t port);
	int endPacket();
	size_t write(uint8_t);
	size_t write(const uint8_t *buffer, size_t size);
	int parsePacket();
	int available();
	int read();
	int read(unsigned char *buffer, size_t len);
	int read(char *buffer, size_t len);
	int peek();
	void flush();
	IPAddress remoteIP();
	uint16_t remotePort();
};

typedef LwIPUDP WiFiUDP;

#endif

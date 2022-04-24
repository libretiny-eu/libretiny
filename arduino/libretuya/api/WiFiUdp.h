#pragma once

#include <Arduino.h>
#include <api/Udp.h>

class IWiFiUDP : public UDP {
  public:
	uint8_t beginMulticast(IPAddress ip, uint16_t port);
	int beginMulticastPacket();
	int beginPacket();
};

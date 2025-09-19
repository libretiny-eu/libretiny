#pragma once

#include <Arduino.h>
#include <api/Udp.h>

class IWiFiUDP : public UDP {
  public:
	IWiFiUDP() {}

	~IWiFiUDP() {}

	virtual uint8_t begin(IPAddress ip, uint16_t port)			= 0;
	virtual uint8_t begin(uint16_t port)						= 0;
	virtual uint8_t beginMulticast(IPAddress ip, uint16_t port) = 0;
	virtual void stop()											= 0;
	virtual int beginMulticastPacket()							= 0;
	virtual int beginPacket()									= 0;
	virtual int beginPacket(IPAddress ip, uint16_t port)		= 0;
	virtual int beginPacket(const char *host, uint16_t port)	= 0;
	virtual int endPacket()										= 0;
	virtual size_t write(uint8_t)								= 0;
	virtual size_t write(const uint8_t *buffer, size_t size)	= 0;
	virtual int parsePacket()									= 0;
	virtual int available()										= 0;
	virtual int read()											= 0;
	virtual int read(unsigned char *buffer, size_t len)			= 0;
	virtual int read(char *buffer, size_t len)					= 0;
	virtual int peek()											= 0;
	virtual void flush()										= 0;
	virtual IPAddress remoteIP()								= 0;
	virtual uint16_t remotePort()								= 0;
};

#if LT_ARD_HAS_WIFI && LT_HAS_LWIP
#include "LwIPUdp.h"
#endif

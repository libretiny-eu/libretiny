/*
  Client.h - Base class that provides Client
  Copyright (c) 2011 Adrian McEwen.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#include <Arduino.h>
#include <api/Client.h>

class IWiFiClient : public Client {
  public:
	IWiFiClient() {}

	IWiFiClient(int sock) {}

	~IWiFiClient() {}

	virtual int connect(IPAddress ip, uint16_t port, int32_t timeout)	  = 0;
	virtual int connect(const char *host, uint16_t port, int32_t timeout) = 0;

	virtual size_t write(Stream &stream) = 0;

	size_t write_P(PGM_P buffer, size_t size) {
		return write((const uint8_t *)buffer, size);
	}

	virtual int fd() const					 = 0;
	virtual int socket()					 = 0;
	virtual int setTimeout(uint32_t seconds) = 0;

	bool operator==(const IWiFiClient &other) const;

	operator bool() {
		return connected();
	}

	virtual bool operator==(const bool value) {
		return bool() == value;
	}

	virtual bool operator!=(const bool value) {
		return bool() != value;
	}

	virtual bool operator!=(const IWiFiClient &other) {
		return !this->operator==(other);
	};

	virtual IPAddress remoteIP() const			= 0;
	virtual IPAddress remoteIP(int sock) const	= 0;
	virtual uint16_t remotePort() const			= 0;
	virtual uint16_t remotePort(int sock) const = 0;
	virtual IPAddress localIP() const			= 0;
	virtual IPAddress localIP(int sock) const	= 0;
	virtual uint16_t localPort() const			= 0;
	virtual uint16_t localPort(int sock) const	= 0;

	using Print::write;
};

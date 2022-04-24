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
#include <memory>

class IWiFiClient : public Client {
  public:
	WiFiClient(int fd);

	int connect(IPAddress ip, uint16_t port, int32_t timeout);
	int connect(const char *host, uint16_t port, int32_t timeout);

	size_t write(Stream &stream);

	int fd() const;
	int socket();
	int setTimeout(uint32_t seconds);

	WiFiClient &operator=(const WiFiClient &other);

	bool operator==(const bool value) {
		return bool() == value;
	}

	bool operator!=(const bool value) {
		return bool() != value;
	}

	bool operator==(const WiFiClient &);

	bool operator!=(const WiFiClient &rhs) {
		return !this->operator==(rhs);
	};

	IPAddress remoteIP() const;
	IPAddress remoteIP(int fd) const;
	uint16_t remotePort() const;
	uint16_t remotePort(int fd) const;
	IPAddress localIP() const;
	IPAddress localIP(int fd) const;
	uint16_t localPort() const;
	uint16_t localPort(int fd) const;

	using Print::write;
};

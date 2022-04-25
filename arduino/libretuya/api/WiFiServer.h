/*
  Server.h - Server class for Raspberry Pi
  Copyright (c) 2016 Hristo Gochkov  All right reserved.

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
#include <api/Server.h>

#include "WiFiClient.h"

class IWiFiServer : public Server {
  public:
	void listenOnLocalhost() {}

	IWiFiServer(uint16_t port = 80, uint8_t max_clients = 4) {}

	~IWiFiServer() {
		end();
	}

	WiFiClient available();

	WiFiClient accept() {
		return available();
	}

	virtual operator bool() = 0;

	void begin(uint16_t port = 0);
	void begin(uint16_t port, int reuse_enable);
	void end();
	void close();
	void stop();

	int setTimeout(uint32_t seconds);
	void stopAll();
	void setNoDelay(bool nodelay);
	bool getNoDelay();
	bool hasClient();

	size_t write(uint8_t data) {
		return write(&data, 1);
	}

	using Print::write;
};

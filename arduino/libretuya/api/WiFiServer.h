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
#include <api/Print.h>

#include "WiFiClient.h"

#include <type_traits>

template <typename TWiFiClient, typename = std::enable_if<std::is_base_of<IWiFiClient, TWiFiClient>::value>>

class IWiFiServer : public Print { // arduino::Server is useless anyway
  public:
	void listenOnLocalhost() {}

	IWiFiServer(uint16_t port = 80, uint8_t maxClients = 4) {}

	IWiFiServer(const IPAddress &addr, uint16_t port = 80, uint8_t maxClients = 4) {}

	~IWiFiServer() {
		stop();
	}

	TWiFiClient available() {
		return accept();
	};

	virtual operator bool() = 0;

	virtual bool begin(uint16_t port = 0, bool reuseAddr = true) = 0;
	virtual void end()											 = 0;
	virtual TWiFiClient accept()								 = 0;

	void close() {
		end();
	}

	void stop() {
		end();
	}

	virtual int setTimeout(uint32_t seconds) = 0;
	virtual void stopAll()					 = 0;
	virtual void setNoDelay(bool noDelay)	 = 0;
	virtual bool getNoDelay()				 = 0;
	virtual bool hasClient()				 = 0;

	size_t write(uint8_t data) {
		return write(&data, 1);
	}

	using Print::write;
};

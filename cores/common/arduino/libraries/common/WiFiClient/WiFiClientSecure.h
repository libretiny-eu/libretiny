/*
  WiFiClientSecure.h - Base class that provides Client SSL to ESP32
  Copyright (c) 2011 Adrian McEwen.  All right reserved.
  Additions Copyright (C) 2017 Evandro Luis Copercini.

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

#include "WiFiClient.h"

class IWiFiClientSecure {
  public:
	virtual int
	connect(IPAddress ip, uint16_t port, const char *rootCABuf, const char *clientCert, const char *clientKey) = 0;
	virtual int
	connect(const char *host, uint16_t port, const char *rootCABuf, const char *clientCert, const char *clientKey) = 0;
	virtual int connect(IPAddress ip, uint16_t port, const char *pskIdent, const char *psk)						   = 0;
	virtual int connect(const char *host, uint16_t port, const char *pskIdent, const char *psk)					   = 0;

	virtual int lastError(char *buf, const size_t size) = 0;
	virtual void setInsecure() = 0; // Don't validate the chain, just accept whatever is given. VERY INSECURE!
	virtual void setPreSharedKey(const char *pskIdent, const char *psk)	 = 0; // psk in hex
	virtual void setCACert(const char *rootCA)							 = 0;
	virtual void setCertificate(const char *clientCA)					 = 0;
	virtual void setPrivateKey(const char *privateKey)					 = 0;
	virtual bool loadCACert(Stream &stream, size_t size)				 = 0;
	virtual bool loadCertificate(Stream &stream, size_t size)			 = 0;
	virtual bool loadPrivateKey(Stream &stream, size_t size)			 = 0;
	virtual bool verify(const char *fingerprint, const char *domainName) = 0;
	virtual void setHandshakeTimeout(unsigned long handshakeTimeout)	 = 0;
	virtual void setAlpnProtocols(const char **alpnProtocols)			 = 0;
	virtual bool getFingerprintSHA256(uint8_t result[32])				 = 0;
};

#if LT_ARD_HAS_WIFI && LT_HAS_MBEDTLS
#include "MbedTLSClient.h"
#endif

/*
  Udp.cpp - UDP class for Raspberry Pi
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

#if LT_ARD_HAS_WIFI && LT_HAS_LWIP

#include "LwIPUdp.h"

extern "C" {

#include <lwip/netdb.h>
#include <lwip/sockets.h>

} // extern "C"

#undef write
#undef read

LwIPUDP::LwIPUDP() : udp_server(-1), server_port(0), remote_port(0), tx_buffer(0), tx_buffer_len(0), rx_buffer(0) {}

LwIPUDP::~LwIPUDP() {
	stop();
}

uint8_t LwIPUDP::begin(IPAddress address, uint16_t port) {
	stop();

	server_port = port;

	tx_buffer = new char[1460];
	if (!tx_buffer) {
		log_e("could not create tx buffer: %d", errno);
		return 0;
	}

	if ((udp_server = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		log_e("could not create socket: %d", errno);
		return 0;
	}

	int yes = 1;
	if (setsockopt(udp_server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
		log_e("could not set socket option: %d", errno);
		stop();
		return 0;
	}

	struct sockaddr_in addr;
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family		 = AF_INET;
	addr.sin_port		 = htons(server_port);
	addr.sin_addr.s_addr = (in_addr_t)address;
	if (bind(udp_server, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		log_e("could not bind socket: %d", errno);
		stop();
		return 0;
	}
	fcntl(udp_server, F_SETFL, O_NONBLOCK);
	return 1;
}

uint8_t LwIPUDP::begin(uint16_t p) {
	return begin(IPAddress(INADDR_ANY), p);
}

uint8_t LwIPUDP::beginMulticast(IPAddress a, uint16_t p) {
	if (begin(IPAddress(INADDR_ANY), p)) {
		if ((uint32_t)a != 0) {
			struct ip_mreq mreq;
			mreq.imr_multiaddr.s_addr = (in_addr_t)a;
			mreq.imr_interface.s_addr = INADDR_ANY;
			if (setsockopt(udp_server, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
				log_e("could not join igmp: %d", errno);
				stop();
				return 0;
			}
			multicast_ip = a;
		}
		return 1;
	}
	return 0;
}

void LwIPUDP::stop() {
	if (tx_buffer) {
		delete[] tx_buffer;
		tx_buffer = NULL;
	}
	tx_buffer_len = 0;
	if (rx_buffer) {
		cbuf *b	  = rx_buffer;
		rx_buffer = NULL;
		delete b;
	}
	if (udp_server == -1)
		return;
	if ((uint32_t)multicast_ip != 0) {
		struct ip_mreq mreq;
		mreq.imr_multiaddr.s_addr = (in_addr_t)multicast_ip;
		mreq.imr_interface.s_addr = (in_addr_t)0;
		setsockopt(udp_server, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
		multicast_ip = IPAddress(INADDR_ANY);
	}
	close(udp_server);
	udp_server = -1;
}

int LwIPUDP::beginMulticastPacket() {
	if (!server_port || multicast_ip == IPAddress(INADDR_ANY))
		return 0;
	remote_ip	= multicast_ip;
	remote_port = server_port;
	return beginPacket();
}

int LwIPUDP::beginPacket() {
	if (!remote_port)
		return 0;

	// allocate tx_buffer if is necessary
	if (!tx_buffer) {
		tx_buffer = new char[1460];
		if (!tx_buffer) {
			log_e("could not create tx buffer: %d", errno);
			return 0;
		}
	}

	tx_buffer_len = 0;

	// check whereas socket is already open
	if (udp_server != -1)
		return 1;

	if ((udp_server = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		log_e("could not create socket: %d", errno);
		return 0;
	}

	fcntl(udp_server, F_SETFL, O_NONBLOCK);

	return 1;
}

int LwIPUDP::beginPacket(IPAddress ip, uint16_t port) {
	remote_ip	= ip;
	remote_port = port;
	return beginPacket();
}

int LwIPUDP::beginPacket(const char *host, uint16_t port) {
	struct hostent *server;
	server = gethostbyname(host);
	if (server == NULL) {
		log_e("could not get host from dns: %d", errno);
		return 0;
	}
	return beginPacket(IPAddress((const uint8_t *)(server->h_addr_list[0])), port);
}

int LwIPUDP::endPacket() {
	struct sockaddr_in recipient;
	recipient.sin_addr.s_addr = (uint32_t)remote_ip;
	recipient.sin_family	  = AF_INET;
	recipient.sin_port		  = htons(remote_port);
	int sent = sendto(udp_server, tx_buffer, tx_buffer_len, 0, (struct sockaddr *)&recipient, sizeof(recipient));
	if (sent < 0) {
		log_e("could not send data: %d", errno);
		return 0;
	}
	return 1;
}

size_t LwIPUDP::write(uint8_t data) {
	if (tx_buffer_len == 1460) {
		endPacket();
		tx_buffer_len = 0;
	}
	tx_buffer[tx_buffer_len++] = data;
	return 1;
}

size_t LwIPUDP::write(const uint8_t *buffer, size_t size) {
	size_t i;
	for (i = 0; i < size; i++)
		write(buffer[i]);
	return i;
}

int LwIPUDP::parsePacket() {
	if (rx_buffer)
		return 0;
	struct sockaddr_in si_other;
	int slen  = sizeof(si_other), len;
	char *buf = new char[1460];
	if (!buf) {
		return 0;
	}
	if ((len = recvfrom(udp_server, buf, 1460, MSG_DONTWAIT, (struct sockaddr *)&si_other, (socklen_t *)&slen)) == -1) {
		delete[] buf;
		if (errno == EWOULDBLOCK) {
			return 0;
		}
		log_e("could not receive data: %d", errno);
		return 0;
	}
	remote_ip	= IPAddress(si_other.sin_addr.s_addr);
	remote_port = ntohs(si_other.sin_port);
	if (len > 0) {
		rx_buffer = new cbuf(len);
		rx_buffer->write(buf, len);
	}
	delete[] buf;
	return len;
}

int LwIPUDP::available() {
	if (!rx_buffer)
		return 0;
	return rx_buffer->available();
}

int LwIPUDP::read() {
	if (!rx_buffer)
		return -1;
	int out = rx_buffer->read();
	if (!rx_buffer->available()) {
		cbuf *b	  = rx_buffer;
		rx_buffer = 0;
		delete b;
	}
	return out;
}

int LwIPUDP::read(unsigned char *buffer, size_t len) {
	return read((char *)buffer, len);
}

int LwIPUDP::read(char *buffer, size_t len) {
	if (!rx_buffer)
		return 0;
	int out = rx_buffer->read(buffer, len);
	if (!rx_buffer->available()) {
		cbuf *b	  = rx_buffer;
		rx_buffer = 0;
		delete b;
	}
	return out;
}

int LwIPUDP::peek() {
	if (!rx_buffer)
		return -1;
	return rx_buffer->peek();
}

void LwIPUDP::flush() {
	if (!rx_buffer)
		return;
	cbuf *b	  = rx_buffer;
	rx_buffer = 0;
	delete b;
}

IPAddress LwIPUDP::remoteIP() {
	return remote_ip;
}

uint16_t LwIPUDP::remotePort() {
	return remote_port;
}

#endif

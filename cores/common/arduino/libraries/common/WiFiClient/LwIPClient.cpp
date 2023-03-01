/* Copyright (c) Kuba Szczodrzyński 2022-04-26. */

#if LT_ARD_HAS_WIFI && LT_HAS_LWIP

#include "LwIPClient.h"

#include <WiFi.h>

#define MAX_SOCK_NUM				4
#define WIFI_CLIENT_CONNECT_TIMEOUT 3000
#define WIFI_CLIENT_READ_TIMEOUT	3000
#define WIFI_CLIENT_WRITE_RETRY		10
#define WIFI_CLIENT_SELECT_TIMEOUT	1000
#define WIFI_CLIENT_FLUSH_BUF_SIZE	1024

// disable #defines removing lwip_ prefix
#undef LWIP_COMPAT_SOCKETS
#define LWIP_COMPAT_SOCKETS 0

extern "C" {

#include <lwip/api.h>
#include <lwip/dns.h>
#include <lwip/err.h>
#include <lwip/sockets.h>
#include <sys/time.h>

} // extern "C"

class SocketHandle {
  public:
	int fd;

	SocketHandle(int fd) : fd(fd) {}

	~SocketHandle() {
		lwip_close(fd);
	}
};

LwIPClient::LwIPClient() {
	LT_VM(CLIENT, "LwIPClient()");
	_connected = false;
	_sock	   = NULL;
	_rxBuffer  = NULL;
	_timeout   = WIFI_CLIENT_CONNECT_TIMEOUT;
}

LwIPClient::LwIPClient(int sock) {
	LT_VM(CLIENT, "LwIPClient(%d)", sock);
	_connected = true;
	_sock	   = std::make_shared<SocketHandle>(sock);
	_rxBuffer  = std::make_shared<LwIPRxBuffer>(sock);
	_timeout   = WIFI_CLIENT_CONNECT_TIMEOUT;
}

LwIPClient::~LwIPClient() {
	LT_VM(CLIENT, "~LwIPClient()");
	stop();
}

LwIPClient &LwIPClient::operator=(const LwIPClient &other) {
	stop();
	_connected = other._connected;
	_sock	   = other._sock;
	_rxBuffer  = other._rxBuffer;
	return *this;
}

bool IWiFiClient::operator==(const IWiFiClient &other) const {
	return fd() == other.fd() && remoteIP() == other.remoteIP() && remotePort() == other.remotePort();
}

int LwIPClient::connect(IPAddress ip, uint16_t port) {
	return connect(ip, port, _timeout);
}

int LwIPClient::connect(const char *host, uint16_t port) {
	return connect(host, port, _timeout);
}

int LwIPClient::connect(const char *host, uint16_t port, int32_t timeout) {
	IPAddress ip = WiFi.hostByName(host);
	if (!ip)
		return 0;
	return connect(ip, port, timeout);
}

int LwIPClient::connect(IPAddress ip, uint16_t port, int32_t timeout) {
	if (_connected)
		stop();
	int sock = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) {
		LT_DM(CLIENT, "socket failed");
		return -1;
	}

	if (timeout <= 0)
		timeout = _timeout; // use default when -1 passed as timeout

	lwip_fcntl(sock, F_SETFL, lwip_fcntl(sock, F_GETFL, 0) | O_NONBLOCK);

	LT_ERRNO();

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family		 = AF_INET;
	addr.sin_addr.s_addr = ip;
	addr.sin_port		 = htons(port);
	fd_set fdset;
	struct timeval tv;
	FD_ZERO(&fdset);
	FD_SET(sock, &fdset);
	tv.tv_sec  = 0;
	tv.tv_usec = timeout * 1000; // millis -> micros

	int res = lwip_connect(sock, (struct sockaddr *)&addr, sizeof(addr));
	if (res < 0 && errno != EINPROGRESS) {
		LT_EM(CLIENT, "Connect failed; errno=%d", errno);
		lwip_close(sock);
		return -1;
	}

	res = lwip_select(sock + 1, NULL, &fdset, NULL, timeout < 0 ? NULL : &tv);
	if (res < 0) {
		LT_EM(CLIENT, "Select failed; errno=%d", errno);
		lwip_close(sock);
		return 0;
	}
	if (res == 0) {
		LT_EM(CLIENT, "Select timeout; errno=%d", errno);
		lwip_close(sock);
		return 0;
	}

	int sockerr;
	socklen_t len = (socklen_t)sizeof(sockerr);
	res			  = lwip_getsockopt(sock, SOL_SOCKET, SO_ERROR, &sockerr, &len);

	if (res < 0 || sockerr != 0) {
		LT_EM(CLIENT, "Socket error; res=%d, sockerr=%d", res, sockerr);
		lwip_close(sock);
		return 0;
	}

	int enable = 1;
	lwip_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	lwip_setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
	lwip_setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
	lwip_setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable));

	LT_ERRNO();

	lwip_fcntl(sock, F_SETFL, lwip_fcntl(sock, F_GETFL, 0) & ~O_NONBLOCK);

	LT_ERRNO();

	_connected = true;
	_sock	   = std::make_shared<SocketHandle>(sock);
	_rxBuffer  = std::make_shared<LwIPRxBuffer>(sock);
	return 1;
}

size_t LwIPClient::write(uint8_t data) {
	return write(&data, 1);
}

size_t LwIPClient::write(Stream &stream) {
	uint8_t *buf = (uint8_t *)malloc(1360);
	if (!buf) {
		return 0;
	}
	size_t toRead = 0, toWrite = 0, written = 0;
	size_t available = stream.available();
	while (available) {
		toRead	= (available > 1360) ? 1360 : available;
		toWrite = stream.readBytes(buf, toRead);
		written += write(buf, toWrite);
		available = stream.available();
	}
	free(buf);
	return written;
}

size_t LwIPClient::write(const uint8_t *buf, size_t size) {
	if (_sock < 0 || !_connected || !size) {
		setWriteError();
		return 0;
	}

	int retry	= WIFI_CLIENT_WRITE_RETRY;
	int written = 0;
	while (retry) {
		fd_set fdset;
		struct timeval tv;
		FD_ZERO(&fdset);
		FD_SET(fd(), &fdset);
		tv.tv_sec  = 0;
		tv.tv_usec = WIFI_CLIENT_SELECT_TIMEOUT * 1000;
		retry--;

		if (lwip_select(fd() + 1, NULL, &fdset, NULL, &tv) < 0) {
			LT_WM(CLIENT, "Select failed; errno=%d", errno);
			return 0;
		}

		if (FD_ISSET(fd(), &fdset)) {
			int res = lwip_send(fd(), buf, size, MSG_DONTWAIT);
			if (res > 0) {
				written += res;
				if (res >= size) {
					retry = 0;
				} else {
					buf += res;
					size -= res;
					retry = WIFI_CLIENT_WRITE_RETRY;
				}
			} else if (res < 0 && errno != EAGAIN) {
				LT_WM(CLIENT, "Send failed; errno=%d", errno);
				setWriteError(res);
				_connected = false;
				retry	   = 0;
			} else {
				// Try again
			}
		}
	}
	LT_DM(CLIENT, "wrote %d bytes", written);
	return written;
}

int LwIPClient::available() {
	if (!_connected || !_rxBuffer)
		return 0;
	int res = _rxBuffer->available();
	if (_rxBuffer->failed()) {
		LT_ERRNO();
		stop();
	}
	return res;
}

int LwIPClient::fd() const {
	if (!_sock)
		return -1;
	return _sock->fd;
}

int LwIPClient::socket() {
	return fd();
}

int LwIPClient::setTimeout(uint32_t seconds) {
	Client::setTimeout(seconds * 1000);
	lwip_setsockopt(fd(), SOL_SOCKET, SO_RCVTIMEO, &_timeout, sizeof(_timeout));
	return lwip_setsockopt(fd(), SOL_SOCKET, SO_SNDTIMEO, &_timeout, sizeof(_timeout));
}

int LwIPClient::read() {
	uint8_t data;
	int res = read(&data, 1);
	if (res < 0)
		return res;
	if (res == 0)
		return -1;
	return data;
}

int LwIPClient::read(uint8_t *buf, size_t size) {
	int res = -1;
	if (_rxBuffer) {
		res = _rxBuffer->read(buf, size);
		if (_rxBuffer->failed()) {
			stop();
		}
	}
	return res;
}

int LwIPClient::peek() {
	int res = -1;
	if (_rxBuffer) {
		res = _rxBuffer->peek();
		if (_rxBuffer->failed()) {
			stop();
		}
	}
	return res;
}

void LwIPClient::flush() {
	int res;
	size_t len = available();
	if (!len)
		return;
	uint8_t *buf = (uint8_t *)malloc(WIFI_CLIENT_FLUSH_BUF_SIZE);
	if (!buf)
		return;
	while (len) {
		res = lwip_recv(fd(), buf, LWIP_MIN(len, WIFI_CLIENT_FLUSH_BUF_SIZE), MSG_DONTWAIT);
		if (res < 0) {
			stop();
			break;
		}
		len -= res;
	}
	free(buf);
}

void LwIPClient::stop() {
	LT_VM(CLIENT, "Stopping TCP");
	_connected = false;
	_sock	   = NULL;
	_rxBuffer  = NULL;
}

uint8_t LwIPClient::connected() {
	if (_connected) {
		uint8_t dummy;
		if (lwip_recv(fd(), &dummy, 0, MSG_DONTWAIT) <= 0) {
			switch (errno) {
				case EWOULDBLOCK:
				case ENOENT: // caused by vfs
				case 0:
					_connected = true;
					break;
				case ENOTCONN:
				case EPIPE:
				case ECONNRESET:
				case ECONNREFUSED:
				case ECONNABORTED:
					LT_IM(CLIENT, "Connection closed; errno=%d", errno);
					_connected = false;
					break;
				default:
					LT_WM(CLIENT, "Connection status unknown; errno=%d", errno);
					_connected = true;
					break;
			}
		}
	}
	return _connected;
}

IPAddress __attribute__((noinline)) getaddr(int sock, int (*func)(int, struct sockaddr *, socklen_t *)) {
	struct sockaddr addr;
	socklen_t len = sizeof(addr);
	func(sock, &addr, &len);
	struct sockaddr_in *s = (struct sockaddr_in *)&addr;
	return IPAddress((uint32_t)(s->sin_addr.s_addr));
}

uint16_t __attribute__((noinline)) getport(int sock, int (*func)(int, struct sockaddr *, socklen_t *)) {
	struct sockaddr addr;
	socklen_t len = sizeof(addr);
	func(sock, &addr, &len);
	struct sockaddr_in *s = (struct sockaddr_in *)&addr;
	return ntohs(s->sin_port);
}

IPAddress LwIPClient::remoteIP() const {
	return getaddr(fd(), lwip_getpeername);
}

IPAddress LwIPClient::remoteIP(int fd) const {
	return getaddr(fd, lwip_getpeername);
}

uint16_t LwIPClient::remotePort() const {
	return getport(fd(), lwip_getpeername);
}

uint16_t LwIPClient::remotePort(int fd) const {
	return getport(fd, lwip_getpeername);
}

IPAddress LwIPClient::localIP() const {
	return getaddr(fd(), lwip_getsockname);
}

IPAddress LwIPClient::localIP(int fd) const {
	return getaddr(fd, lwip_getsockname);
}

uint16_t LwIPClient::localPort() const {
	return getport(fd(), lwip_getsockname);
}

uint16_t LwIPClient::localPort(int fd) const {
	return getport(fd, lwip_getsockname);
}

#endif

#include "WiFiClient.h"
#include "WiFiPriv.h"

WiFiClient::WiFiClient() {
	DiagPrintf("WiFiClient()\r\n");
	_sock	   = -1;
	_connected = false;
	_rxBuffer  = NULL;
	_timeout   = WIFI_CLIENT_CONNECT_TIMEOUT;
}

WiFiClient::WiFiClient(int sock) {
	_sock	   = sock;
	_connected = true;
	_rxBuffer  = new LwIPRxBuffer(sock);
	_timeout   = WIFI_CLIENT_CONNECT_TIMEOUT;
}

WiFiClient &WiFiClient::operator=(const IWiFiClient &other) {
	stop();
	WiFiClient *oth = (WiFiClient *)&other;
	_sock			= oth->_sock;
	_connected		= oth->_connected;
	_rxBuffer		= oth->_rxBuffer;
	return *this;
}

bool WiFiClient::operator==(const IWiFiClient &other) const {
	return _sock == other.fd() && remoteIP() == other.remoteIP() && remotePort() == other.remotePort();
}

int WiFiClient::connect(IPAddress ip, uint16_t port) {
	return connect(ip, port, _timeout);
}

int WiFiClient::connect(const char *host, uint16_t port) {
	return connect(host, port, _timeout);
}

int WiFiClient::connect(const char *host, uint16_t port, int32_t timeout) {
	IPAddress ip = WiFi.hostByName(host);
	if (!ip)
		return 0;
	return connect(ip, port, timeout);
}

int WiFiClient::connect(IPAddress ip, uint16_t port, int32_t timeout) {
	int sock = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) {
		return -1;
	}

	lwip_fcntl(sock, F_SETFL, lwip_fcntl(sock, F_GETFL, 0) | O_NONBLOCK);

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
	tv.tv_usec = timeout * 1000;

	int res = lwip_connect(sock, (struct sockaddr *)&addr, sizeof(addr));
	if (res < 0 && errno != EINPROGRESS) {
		printf("lwip_connect() errno %d\r\n", errno);
		lwip_close(sock);
		return -1;
	}

	res = lwip_select(sock + 1, NULL, &fdset, NULL, timeout < 0 ? NULL : &tv);
	if (res < 0) {
		printf("lwip_select() errno %d\r\n", errno);
		lwip_close(sock);
		return 0;
	}
	if (res == 0) {
		printf("lwip_select() timeout errno %d\r\n", errno);
		lwip_close(sock);
		return 0;
	}

	int sockerr;
	socklen_t len = (socklen_t)sizeof(sockerr);
	res			  = lwip_getsockopt(sock, SOL_SOCKET, SO_ERROR, &sockerr, &len);

	if (res < 0 || sockerr != 0) {
		lwip_close(sock);
		return 0;
	}

	int enable = 1;
	lwip_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	lwip_setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
	lwip_setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
	lwip_setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable));

	lwip_fcntl(sock, F_SETFL, lwip_fcntl(sock, F_GETFL, 0) & ~O_NONBLOCK);

	if (_sock != -1)
		lwip_close(_sock);
	_sock	   = sock;
	_connected = true;
	free(_rxBuffer);
	_rxBuffer = new LwIPRxBuffer(_sock);
	return 1;
}

size_t WiFiClient::write(uint8_t data) {
	return write(&data, 1);
}

size_t WiFiClient::write(Stream &stream) {
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

size_t WiFiClient::write(const uint8_t *buf, size_t size) {
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
		FD_SET(_sock, &fdset);
		tv.tv_sec  = 0;
		tv.tv_usec = WIFI_CLIENT_SELECT_TIMEOUT * 1000;
		retry--;

		if (lwip_select(_sock + 1, NULL, &fdset, NULL, &tv) < 0) {
			return 0;
		}

		if (FD_ISSET(_sock, &fdset)) {
			int res = lwip_send(_sock, buf, size, MSG_DONTWAIT);
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
				setWriteError(res);
				_connected = false;
				retry	   = 0;
			} else {
				// Try again
			}
		}
	}
	return written;
}

int WiFiClient::available() {
	if (!_connected)
		return 0;
	int res = _rxBuffer->available();
	if (_rxBuffer->failed()) {
		stop();
	}
	return res;
}

int WiFiClient::fd() const {
	return _sock;
}

int WiFiClient::socket() {
	return _sock;
}

int WiFiClient::setTimeout(uint32_t seconds) {
	Client::setTimeout(seconds * 1000);
	lwip_setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, &_timeout, sizeof(_timeout));
	return lwip_setsockopt(_sock, SOL_SOCKET, SO_SNDTIMEO, &_timeout, sizeof(_timeout));
}

int WiFiClient::read() {
	uint8_t data;
	int res = read(&data, 1);
	if (res < 0)
		return res;
	if (res == 0)
		return -1;
	return data;
}

int WiFiClient::read(uint8_t *buf, size_t size) {
	int res = -1;
	if (_rxBuffer) {
		res = _rxBuffer->read(buf, size);
		if (_rxBuffer->failed()) {
			stop();
		}
	}
	return res;
}

int WiFiClient::peek() {
	int res = -1;
	if (_rxBuffer) {
		res = _rxBuffer->peek();
		if (_rxBuffer->failed()) {
			stop();
		}
	}
	return res;
}

void WiFiClient::flush() {
	int res;
	size_t len = available();
	if (!len)
		return;
	uint8_t *buf = (uint8_t *)malloc(WIFI_CLIENT_FLUSH_BUF_SIZE);
	if (!buf)
		return;
	while (len) {
		res = lwip_recv(_sock, buf, LWIP_MIN(len, WIFI_CLIENT_FLUSH_BUF_SIZE), MSG_DONTWAIT);
		if (res < 0) {
			stop();
			break;
		}
		len -= res;
	}
	free(buf);
}

void WiFiClient::stop() {
	if (_sock != -1)
		lwip_close(_sock);
	_sock	   = -1;
	_connected = false;
	free(_rxBuffer);
	_rxBuffer = NULL;
}

uint8_t WiFiClient::connected() {
	if (_connected) {
		uint8_t dummy;
		if (lwip_recv(_sock, &dummy, 0, MSG_DONTWAIT) < 0) {
			switch (errno) {
				case EWOULDBLOCK:
				case ENOENT: // caused by vfs
					_connected = true;
					break;
				case ENOTCONN:
				case EPIPE:
				case ECONNRESET:
				case ECONNREFUSED:
				case ECONNABORTED:
					_connected = false;
					break;
				default:
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

IPAddress WiFiClient::remoteIP() const {
	return getaddr(_sock, lwip_getpeername);
}

IPAddress WiFiClient::remoteIP(int fd) const {
	return getaddr(fd, lwip_getpeername);
}

uint16_t WiFiClient::remotePort() const {
	return getport(_sock, lwip_getpeername);
}

uint16_t WiFiClient::remotePort(int fd) const {
	return getport(fd, lwip_getpeername);
}

IPAddress WiFiClient::localIP() const {
	return getaddr(_sock, lwip_getsockname);
}

IPAddress WiFiClient::localIP(int fd) const {
	return getaddr(fd, lwip_getsockname);
}

uint16_t WiFiClient::localPort() const {
	return getport(_sock, lwip_getsockname);
}

uint16_t WiFiClient::localPort(int fd) const {
	return getport(fd, lwip_getsockname);
}

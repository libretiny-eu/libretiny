#include "WiFi.h"
#include "WiFiSSLClient.h"

extern "C" {
    #include "wl_definitions.h"
    #include "wl_types.h"
    #include "string.h"
    #include "errno.h"
}

WiFiSSLClient::WiFiSSLClient() {
    _is_connected = false;
    _sock = -1;

    sslclient.socket = -1;
    sslclient.ssl = NULL;
    sslclient.recvTimeout = 3000;

    _rootCABuff = NULL;
    _cli_cert = NULL;
    _cli_key = NULL;
    _psKey = NULL;
    _pskIdent = NULL;
    _sni_hostname = NULL;
}

WiFiSSLClient::WiFiSSLClient(uint8_t sock) {
    _sock = sock;

    sslclient.socket = -1;
    sslclient.ssl = NULL;
    sslclient.recvTimeout = 3000;

//    if(sock >= 0) {
//        _is_connected = true;
//    }
    _is_connected = true;

    _rootCABuff = NULL;
    _cli_cert = NULL;
    _cli_key = NULL;
    _psKey = NULL;
    _pskIdent = NULL;
    _sni_hostname = NULL;
}

uint8_t WiFiSSLClient::connected() {
    if (sslclient.socket < 0) {
        _is_connected = false;
        return 0;
    } else {
        if (_is_connected) {
            return 1;
        } else {
            stop();
            return 0;
        }
    }
}

int WiFiSSLClient::available() {
    int ret = 0;
    int err;

    if (!_is_connected) {
        return 0;
    }
    if (sslclient.socket >= 0) {
        ret = ssldrv.availData(&sslclient);
        if (ret > 0) {
            return 1;
        } else {
            err = ssldrv.getLastErrno(&sslclient);
            if ((err > 0) && (err != EAGAIN)) {
                _is_connected = false;
            }
        }
        return 0;
    }

    return 0;
}

int WiFiSSLClient::read() {
    int ret;
    int err;
    uint8_t b[1];

    if (!available()) {
        return -1;
    }

    ret = ssldrv.getData(&sslclient, b);
    if (ret > 0) {
        return b[0];
    } else {
        err = ssldrv.getLastErrno(&sslclient);
        if ((err > 0) && (err != EAGAIN)) {
            _is_connected = false;
        }
    }
    return -1;
}

int WiFiSSLClient::read(uint8_t* buf, size_t size) {
    uint16_t _size = size;
    int ret;
    int err;

    ret = ssldrv.getDataBuf(&sslclient, buf, _size);
    if (ret <= 0) {
        err = ssldrv.getLastErrno(&sslclient);
        if ((err > 0) && (err != EAGAIN)) {
            _is_connected = false;
        }
    }
    return ret;
}

void WiFiSSLClient::stop() {

    if (sslclient.socket < 0) {
        return;
    }

    ssldrv.stopClient(&sslclient);
    _is_connected = false;

    sslclient.socket = -1;
    _sock = -1;
}

size_t WiFiSSLClient::write(uint8_t b) {
    return write(&b, 1);
}

size_t WiFiSSLClient::write(const uint8_t *buf, size_t size) {
    if (sslclient.socket < 0) {
        setWriteError();
        return 0;
    }
    if (size == 0) {
        setWriteError();
        return 0;
    }

    if (!ssldrv.sendData(&sslclient, buf, size)) {
        setWriteError();
        _is_connected = false;
        return 0;
    }

    return size;
}

WiFiSSLClient::operator bool() {
    return (sslclient.socket >= 0);
}

int WiFiSSLClient::connect(IPAddress ip, uint16_t port) {
    if (_psKey != NULL && _pskIdent != NULL)
        return connect(ip, port, _pskIdent, _psKey);
    return connect(ip, port, _rootCABuff, _cli_cert, _cli_key);
}

int WiFiSSLClient::connect(const char *host, uint16_t port) {

    if (_sni_hostname == NULL) {
        _sni_hostname = (char*)host;
    }

    if (_psKey != NULL && _pskIdent != NULL)
        return connect(host, port, _pskIdent, _psKey);
    return connect(host, port, _rootCABuff, _cli_cert, _cli_key);
}

int WiFiSSLClient::connect(const char* host, uint16_t port, unsigned char* rootCABuff, unsigned char* cli_cert, unsigned char* cli_key) {
    IPAddress remote_addr;

    if (_sni_hostname == NULL) {
        _sni_hostname = (char*)host;
    }

    if (WiFi.hostByName(host, remote_addr)) {
        return connect(remote_addr, port, rootCABuff, cli_cert, cli_key);
    }
    return 0;
}

int WiFiSSLClient::connect(IPAddress ip, uint16_t port, unsigned char* rootCABuff, unsigned char* cli_cert, unsigned char* cli_key) {
    int ret = 0;

    ret = ssldrv.startClient(&sslclient, uint32_t(ip), port, rootCABuff, cli_cert, cli_key, NULL, NULL, _sni_hostname);

    if (ret < 0) {
        _is_connected = false;
        return 0;
    } else {
        _is_connected = true;
    }

    return 1;
}

int WiFiSSLClient::connect(const char *host, uint16_t port, unsigned char* pskIdent, unsigned char* psKey) {
    IPAddress remote_addr;

    if (_sni_hostname == NULL) {
        _sni_hostname = (char*)host;
    }

    if (WiFi.hostByName(host, remote_addr)) {
        return connect(remote_addr, port, pskIdent, psKey);
    }
    return 0;
}

int WiFiSSLClient::connect(IPAddress ip, uint16_t port, unsigned char* pskIdent, unsigned char* psKey) {
    int ret = 0;

    ret = ssldrv.startClient(&sslclient, uint32_t(ip), port, NULL, NULL, NULL, pskIdent, psKey, _sni_hostname);

    if (ret < 0) {
        _is_connected = false;
        return 0;
    } else {
        _is_connected = true;
    }

    return 1;
}

int WiFiSSLClient::peek() {
    uint8_t b;

    if (!available()) {
        return -1;
    }

    ssldrv.getData(&sslclient, &b, 1);

    return b;
}
void WiFiSSLClient::flush() {
    while (available()) {
        read();
    }
}

void WiFiSSLClient::setRootCA(unsigned char *rootCA) {
    _rootCABuff = rootCA;
}

void WiFiSSLClient::setClientCertificate(unsigned char *client_ca, unsigned char *private_key) {
    _cli_cert = client_ca;
    _cli_key = private_key;
}

void WiFiSSLClient::setPreSharedKey(unsigned char *pskIdent, unsigned char *psKey) {
    _psKey = psKey;
    _pskIdent = pskIdent;
}

int WiFiSSLClient::setRecvTimeout(int timeout) {
    sslclient.recvTimeout = timeout;
    if (connected()) {
        ssldrv.setSockRecvTimeout(sslclient.socket, sslclient.recvTimeout);
    }

    return 0;
}

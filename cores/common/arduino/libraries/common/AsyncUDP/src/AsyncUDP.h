#ifndef ESPASYNCUDP_H
#define ESPASYNCUDP_H

#ifndef CONFIG_TCP_MSS
#define CONFIG_TCP_MSS 1436
#endif

#include "IPAddress.h"
#include "Print.h"
#include "Stream.h"
#include <functional>
extern "C" {
#include "lwip/ip_addr.h"
}
class AsyncUDP;
class AsyncUDPPacket;
class AsyncUDPMessage;
struct udp_pcb;
struct pbuf;

typedef std::function<void(AsyncUDPPacket& packet)> AuPacketHandlerFunction;
typedef std::function<void(void * arg, AsyncUDPPacket& packet)> AuPacketHandlerFunctionWithArg;

class AsyncUDPMessage : public Print
{
protected:
    uint8_t *_buffer;
    size_t _index;
    size_t _size;
public:
    AsyncUDPMessage(size_t size=CONFIG_TCP_MSS);
    virtual ~AsyncUDPMessage();
    size_t write(const uint8_t *data, size_t len);
    size_t write(uint8_t data);
    size_t space();
    uint8_t * data();
    size_t length();
    void flush();
    operator bool()
    {
        return _buffer != NULL;
    }
};

class AsyncUDPPacket : public Stream
{
protected:
    AsyncUDP *_udp;
    pbuf *_pb;
    ip_addr_t _localIp;
    uint16_t _localPort;
    ip_addr_t _remoteIp;
    uint16_t _remotePort;
    uint8_t _remoteMac[6];
    uint8_t *_data;
    size_t _len;
    size_t _index;
public:
    AsyncUDPPacket(AsyncUDPPacket &packet);
    AsyncUDPPacket(AsyncUDP *udp, pbuf *pb, const ip_addr_t *addr, uint16_t port);
    virtual ~AsyncUDPPacket();

    uint8_t * data();
    size_t length();
    bool isBroadcast();
    bool isMulticast();

    IPAddress localIP();
    uint16_t localPort();
    IPAddress remoteIP();
    uint16_t remotePort();
    void remoteMac(uint8_t * mac);

    size_t send(AsyncUDPMessage &message);

    int available();
    size_t read(uint8_t *data, size_t len);
    int read();
    int peek();
    void flush();

    size_t write(const uint8_t *data, size_t len);
    size_t write(uint8_t data);
};

class AsyncUDP : public Print
{
protected:
    udp_pcb *_pcb;
    //SemaphoreHandle_t _lock;
    bool _connected;
	esp_err_t _lastErr;
    AuPacketHandlerFunction _handler;

    bool _init();
    void _recv(udp_pcb *upcb, pbuf *pb, const ip_addr_t *addr, uint16_t port);

public:
    AsyncUDP();
    virtual ~AsyncUDP();

    void onPacket(AuPacketHandlerFunctionWithArg cb, void * arg=NULL);
    void onPacket(AuPacketHandlerFunction cb);

    bool listen(const ip_addr_t *addr, uint16_t port);
    bool listen(const IPAddress addr, uint16_t port);
    bool listen(uint16_t port);

    bool listenMulticast(const ip_addr_t *addr, uint16_t port, uint8_t ttl=1);
    bool listenMulticast(const IPAddress addr, uint16_t port, uint8_t ttl=1);

    bool connect(const ip_addr_t *addr, uint16_t port);
    bool connect(const IPAddress addr, uint16_t port);

    void close();

    size_t writeTo(const uint8_t *data, size_t len, const ip_addr_t *addr, uint16_t port);
    size_t writeTo(const uint8_t *data, size_t len, const IPAddress addr, uint16_t port);
    size_t write(const uint8_t *data, size_t len);
    size_t write(uint8_t data);

    size_t broadcastTo(uint8_t *data, size_t len, uint16_t port);
    size_t broadcastTo(const char * data, uint16_t port);
    size_t broadcast(uint8_t *data, size_t len);
    size_t broadcast(const char * data);

    size_t sendTo(AsyncUDPMessage &message, const ip_addr_t *addr, uint16_t port);
    size_t sendTo(AsyncUDPMessage &message, const IPAddress addr, uint16_t port);
    size_t send(AsyncUDPMessage &message);

    size_t broadcastTo(AsyncUDPMessage &message, uint16_t port);
    size_t broadcast(AsyncUDPMessage &message);

    IPAddress listenIP();
    bool connected();
	esp_err_t lastErr();
    operator bool();

    static void _s_recv(void *arg, udp_pcb *upcb, pbuf *p, const ip_addr_t *addr, uint16_t port);
};

#endif

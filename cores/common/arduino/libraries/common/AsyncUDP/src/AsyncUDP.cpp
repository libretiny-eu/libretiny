#include "Arduino.h"
#include "AsyncUDP.h"

extern "C" {
#include "lwip/opt.h"
#include "lwip/inet.h"
#include "lwip/udp.h"
#include "lwip/igmp.h"
#include "lwip/ip_addr.h"
#include "lwip/mld6.h"
#include "lwip/prot/ethernet.h"
}

#include "lwip/priv/tcpip_priv.h"

typedef struct {
    struct tcpip_api_call_data call;
    udp_pcb * pcb;
    const ip_addr_t *addr;
    uint16_t port;
    struct pbuf *pb;
    err_t err;
} udp_api_call_t;

static err_t _udp_connect_api(struct tcpip_api_call_data *api_call_msg){
    udp_api_call_t * msg = (udp_api_call_t *)api_call_msg;
    msg->err = udp_connect(msg->pcb, msg->addr, msg->port);
    return msg->err;
}

static err_t _udp_connect(struct udp_pcb *pcb, const ip_addr_t *addr, u16_t port){
    udp_api_call_t msg;
    msg.pcb = pcb;
    msg.addr = addr;
    msg.port = port;
    tcpip_api_call(_udp_connect_api, (struct tcpip_api_call_data*)&msg);
    return msg.err;
}

static err_t _udp_disconnect_api(struct tcpip_api_call_data *api_call_msg){
    udp_api_call_t * msg = (udp_api_call_t *)api_call_msg;
    msg->err = 0;
    udp_disconnect(msg->pcb);
    return msg->err;
}

static void  _udp_disconnect(struct udp_pcb *pcb){
    udp_api_call_t msg;
    msg.pcb = pcb;
    tcpip_api_call(_udp_disconnect_api, (struct tcpip_api_call_data*)&msg);
}

static err_t _udp_remove_api(struct tcpip_api_call_data *api_call_msg){
    udp_api_call_t * msg = (udp_api_call_t *)api_call_msg;
    msg->err = 0;
    udp_remove(msg->pcb);
    return msg->err;
}

static void  _udp_remove(struct udp_pcb *pcb){
    udp_api_call_t msg;
    msg.pcb = pcb;
    tcpip_api_call(_udp_remove_api, (struct tcpip_api_call_data*)&msg);
}

static err_t _udp_bind_api(struct tcpip_api_call_data *api_call_msg){
    udp_api_call_t * msg = (udp_api_call_t *)api_call_msg;
    msg->err = udp_bind(msg->pcb, msg->addr, msg->port);
    return msg->err;
}

static err_t _udp_bind(struct udp_pcb *pcb, const ip_addr_t *addr, u16_t port){
    udp_api_call_t msg;
    msg.pcb = pcb;
    msg.addr = addr;
    msg.port = port;
    tcpip_api_call(_udp_bind_api, (struct tcpip_api_call_data*)&msg);
    return msg.err;
}

static err_t _udp_sendto_api(struct tcpip_api_call_data *api_call_msg){
    udp_api_call_t * msg = (udp_api_call_t *)api_call_msg;
    msg->err = udp_sendto(msg->pcb, msg->pb, msg->addr, msg->port);
    return msg->err;
}

static err_t _udp_sendto(struct udp_pcb *pcb, struct pbuf *pb, const ip_addr_t *addr, u16_t port){
    udp_api_call_t msg;
    msg.pcb = pcb;
    msg.addr = addr;
    msg.port = port;
    msg.pb = pb;
    tcpip_api_call(_udp_sendto_api, (struct tcpip_api_call_data*)&msg);
    return msg.err;
}

typedef struct {
        void *arg;
        udp_pcb *pcb;
        pbuf *pb;
        const ip_addr_t *addr;
        uint16_t port;
} lwip_event_packet_t;

static QueueHandle_t _udp_queue;
static volatile TaskHandle_t _udp_task_handle = NULL;

static void _udp_task(void *pvParameters){
    lwip_event_packet_t * e = NULL;
    for (;;) {
        if(xQueueReceive(_udp_queue, &e, portMAX_DELAY) == pdTRUE){
            if(!e->pb){
                free((void*)(e));
                continue;
            }
            AsyncUDP::_s_recv(e->arg, e->pcb, e->pb, e->addr, e->port);
            free((void*)(e));
        }
    }
    _udp_task_handle = NULL;
    vTaskDelete(NULL);
}

static bool _udp_task_start(){
    if(!_udp_queue){
        _udp_queue = xQueueCreate(32, sizeof(lwip_event_packet_t *));
        if(!_udp_queue){
            return false;
        }
    }
    if(!_udp_task_handle){
        xTaskCreateUniversal(_udp_task, "async_udp", 4096, NULL, /* CONFIG_ARDUINO_UDP_TASK_PRIORITY */ 3, (TaskHandle_t*)&_udp_task_handle, /* CONFIG_ARDUINO_UDP_RUNNING_CORE*/ 0);
        if(!_udp_task_handle){
            return false;
        }
    }
    return true;
}

static bool _udp_task_post(void *arg, udp_pcb *pcb, pbuf *pb, const ip_addr_t *addr, uint16_t port)
{
    if(!_udp_task_handle || !_udp_queue){
        return false;
    }
    lwip_event_packet_t * e = (lwip_event_packet_t *)malloc(sizeof(lwip_event_packet_t));
    if(!e){
        return false;
    }
    e->arg = arg;
    e->pcb = pcb;
    e->pb = pb;
    e->addr = addr;
    e->port = port;
    if (xQueueSend(_udp_queue, &e, portMAX_DELAY) != pdPASS) {
        free((void*)(e));
        return false;
    }
    return true;
}

static void _udp_recv(void *arg, udp_pcb *pcb, pbuf *pb, const ip_addr_t *addr, uint16_t port)
{
    while(pb != NULL) {
        pbuf * this_pb = pb;
        pb = pb->next;
        this_pb->next = NULL;
        if(!_udp_task_post(arg, pcb, this_pb, addr, port)){
            pbuf_free(this_pb);
        }
    }
}
/*
static bool _udp_task_stop(){
    if(!_udp_task_post(NULL, NULL, NULL, NULL, 0, NULL)){
        return false;
    }
    while(_udp_task_handle){
        vTaskDelay(10);
    }

    lwip_event_packet_t * e;
    while (xQueueReceive(_udp_queue, &e, 0) == pdTRUE) {
        if(e->pb){
            pbuf_free(e->pb);
        }
        free((void*)(e));
    }
    vQueueDelete(_udp_queue);
    _udp_queue = NULL;
}
*/



#define UDP_MUTEX_LOCK()    //xSemaphoreTake(_lock, portMAX_DELAY)
#define UDP_MUTEX_UNLOCK()  //xSemaphoreGive(_lock)


AsyncUDPMessage::AsyncUDPMessage(size_t size)
{
    _index = 0;
    if(size > CONFIG_TCP_MSS) {
        size = CONFIG_TCP_MSS;
    }
    _size = size;
    _buffer = (uint8_t *)malloc(size);
}

AsyncUDPMessage::~AsyncUDPMessage()
{
    if(_buffer) {
        free(_buffer);
    }
}

size_t AsyncUDPMessage::write(const uint8_t *data, size_t len)
{
    if(_buffer == NULL) {
        return 0;
    }
    size_t s = space();
    if(len > s) {
        len = s;
    }
    memcpy(_buffer + _index, data, len);
    _index += len;
    return len;
}

size_t AsyncUDPMessage::write(uint8_t data)
{
    return write(&data, 1);
}

size_t AsyncUDPMessage::space()
{
    if(_buffer == NULL) {
        return 0;
    }
    return _size - _index;
}

uint8_t * AsyncUDPMessage::data()
{
    return _buffer;
}

size_t AsyncUDPMessage::length()
{
    return _index;
}

void AsyncUDPMessage::flush()
{
    _index = 0;
}

AsyncUDPPacket::AsyncUDPPacket(AsyncUDPPacket &packet){
    _udp = packet._udp;
    _pb = packet._pb;
    _data = packet._data;
    _len = packet._len;
    _index = 0;

    memcpy(&_remoteIp, &packet._remoteIp, sizeof(ip_addr_t));
    memcpy(&_localIp, &packet._localIp, sizeof(ip_addr_t));
    _localPort = packet._localPort;
    _remotePort = packet._remotePort;
    memcpy(_remoteMac, packet._remoteMac, 6);

    pbuf_ref(_pb);
}

AsyncUDPPacket::AsyncUDPPacket(AsyncUDP *udp, pbuf *pb, const ip_addr_t *raddr, uint16_t rport)
{
    _udp = udp;
    _pb = pb;
    _data = (uint8_t*)(pb->payload);
    _len = pb->len;
    _index = 0;

    pbuf_ref(_pb);

    //memcpy(&_remoteIp, raddr, sizeof(ip_addr_t));
    eth_hdr* eth = NULL;
    udp_hdr* udphdr = (udp_hdr *)(_data - UDP_HLEN);
    _localPort = ntohs(udphdr->dest);
    _remotePort = ntohs(udphdr->src);

    eth = (eth_hdr *)(_data - UDP_HLEN - IP_HLEN - SIZEOF_ETH_HDR);
    struct ip_hdr * iphdr = (struct ip_hdr *)(_data - UDP_HLEN - IP_HLEN);
    _localIp.addr = iphdr->dest.addr;
    _remoteIp.addr = iphdr->src.addr;

    memcpy(_remoteMac, eth->src.addr, 6);
}

AsyncUDPPacket::~AsyncUDPPacket()
{
    pbuf_free(_pb);
}

uint8_t * AsyncUDPPacket::data()
{
    return _data;
}

size_t AsyncUDPPacket::length()
{
    return _len;
}

int AsyncUDPPacket::available(){
    return _len - _index;
}

size_t AsyncUDPPacket::read(uint8_t *data, size_t len){
    size_t i;
    size_t a = _len - _index;
    if(len > a){
        len = a;
    }
    for(i=0;i<len;i++){
        data[i] = read();
    }
    return len;
}

int AsyncUDPPacket::read(){
    if(_index < _len){
        return _data[_index++];
    }
    return -1;
}

int AsyncUDPPacket::peek(){
    if(_index < _len){
        return _data[_index];
    }
    return -1;
}

void AsyncUDPPacket::flush(){
    _index = _len;
}

IPAddress AsyncUDPPacket::localIP()
{
    return IPAddress(_localIp.addr);
}

uint16_t AsyncUDPPacket::localPort()
{
    return _localPort;
}

IPAddress AsyncUDPPacket::remoteIP()
{
    return IPAddress(_remoteIp.addr);
}

uint16_t AsyncUDPPacket::remotePort()
{
    return _remotePort;
}

void AsyncUDPPacket::remoteMac(uint8_t * mac)
{
    memcpy(mac, _remoteMac, 6);
}

bool AsyncUDPPacket::isBroadcast()
{
    uint32_t ip = _localIp.addr;
    return ip == 0xFFFFFFFF || ip == 0 || (ip & 0xFF000000) == 0xFF000000;
}

bool AsyncUDPPacket::isMulticast()
{
    return ip_addr_ismulticast(&(_localIp));
}

size_t AsyncUDPPacket::write(const uint8_t *data, size_t len)
{
    if(!data){
        return 0;
    }
    return _udp->writeTo(data, len, &_remoteIp, _remotePort);
}

size_t AsyncUDPPacket::write(uint8_t data)
{
    return write(&data, 1);
}

size_t AsyncUDPPacket::send(AsyncUDPMessage &message)
{
    return write(message.data(), message.length());
}

bool AsyncUDP::_init(){
    if(_pcb){
        return true;
    }
    _pcb = udp_new();
    if(!_pcb){
        return false;
    }
    //_lock = xSemaphoreCreateMutex();
    udp_recv(_pcb, &_udp_recv, (void *) this);
    return true;
}

AsyncUDP::AsyncUDP()
{
    _pcb = NULL;
    _connected = false;
	_lastErr = ERR_OK;
    _handler = NULL;
}

AsyncUDP::~AsyncUDP()
{
    close();
    UDP_MUTEX_LOCK();
    udp_recv(_pcb, NULL, NULL);
    _udp_remove(_pcb);
    _pcb = NULL;
    UDP_MUTEX_UNLOCK();
    //vSemaphoreDelete(_lock);
}

void AsyncUDP::close()
{
    UDP_MUTEX_LOCK();
    if(_pcb != NULL) {
        if(_connected) {
            _udp_disconnect(_pcb);
        }
        _connected = false;
        //todo: unjoin multicast group
    }
    UDP_MUTEX_UNLOCK();
}

bool AsyncUDP::connect(const ip_addr_t *addr, uint16_t port)
{
    if(!_udp_task_start()){
        log_e("failed to start task");
        return false;
    }
    if(!_init()) {
        return false;
    }
    close();
    UDP_MUTEX_LOCK();
    _lastErr = _udp_connect(_pcb, addr, port);
    if(_lastErr != ERR_OK) {
        UDP_MUTEX_UNLOCK();
        return false;
    }
    _connected = true;
    UDP_MUTEX_UNLOCK();
    return true;
}

bool AsyncUDP::listen(const ip_addr_t *addr, uint16_t port)
{
    if(!_udp_task_start()){
        log_e("failed to start task");
        return false;
    }
    if(!_init()) {
        return false;
    }
    close();
    if(addr){
        IP_SET_TYPE_VAL(_pcb->local_ip,  addr->type);
        IP_SET_TYPE_VAL(_pcb->remote_ip, addr->type);
    }
    UDP_MUTEX_LOCK();
    if(_udp_bind(_pcb, addr, port) != ERR_OK) {
        UDP_MUTEX_UNLOCK();
        return false;
    }
    _connected = true;
    UDP_MUTEX_UNLOCK();
    return true;
}

static esp_err_t joinMulticastGroup(const ip_addr_t *addr, bool join=true)
{
    if(join){
        if (igmp_joingroup(IP4_ADDR_ANY, addr)) {
            return ESP_ERR_INVALID_STATE;
        }
    } else {
        if (igmp_leavegroup(IP4_ADDR_ANY, addr)) {
            return ESP_ERR_INVALID_STATE;
        }
    }

    return ESP_OK;
}

bool AsyncUDP::listenMulticast(const ip_addr_t *addr, uint16_t port, uint8_t ttl)
{
    if(!ip_addr_ismulticast(addr)) {
        return false;
    }

    if (joinMulticastGroup(addr, true)!= ERR_OK) {
        return false;
    }

    if(!listen(NULL, port)) {
        return false;
    }

    UDP_MUTEX_LOCK();
    _pcb->mcast_ttl = ttl;
    _pcb->remote_port = port;
    ip_addr_copy(_pcb->remote_ip, *addr);
    //ip_addr_copy(_pcb->remote_ip, ip_addr_any_type);
    UDP_MUTEX_UNLOCK();

    return true;
}

size_t AsyncUDP::writeTo(const uint8_t * data, size_t len, const ip_addr_t * addr, uint16_t port)
{
    if(!_pcb) {
        UDP_MUTEX_LOCK();
        _pcb = udp_new();
        UDP_MUTEX_UNLOCK();
        if(_pcb == NULL) {
            return 0;
        }
    }
    if(len > CONFIG_TCP_MSS) {
        len = CONFIG_TCP_MSS;
    }
    _lastErr = ERR_OK;
    pbuf* pbt = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    if(pbt != NULL) {
        uint8_t* dst = reinterpret_cast<uint8_t*>(pbt->payload);
        memcpy(dst, data, len);
        UDP_MUTEX_LOCK();
        _lastErr = _udp_sendto(_pcb, pbt, addr, port);
        UDP_MUTEX_UNLOCK();
        pbuf_free(pbt);
        if(_lastErr < ERR_OK) {
            return 0;
        }
        return len;
    }
    return 0;
}

void AsyncUDP::_recv(udp_pcb *upcb, pbuf *pb, const ip_addr_t *addr, uint16_t port)
{
    while(pb != NULL) {
        pbuf * this_pb = pb;
        pb = pb->next;
        this_pb->next = NULL;
        if(_handler) {
            AsyncUDPPacket packet(this, this_pb, addr, port);
            _handler(packet);
        }
        pbuf_free(this_pb);
    }
}

void AsyncUDP::_s_recv(void *arg, udp_pcb *upcb, pbuf *p, const ip_addr_t *addr, uint16_t port)
{
    reinterpret_cast<AsyncUDP*>(arg)->_recv(upcb, p, addr, port);
}

bool AsyncUDP::listen(uint16_t port)
{
    return listen(IP_ANY_TYPE, port);
}

bool AsyncUDP::listen(const IPAddress addr, uint16_t port)
{
    ip_addr_t laddr;
    laddr.addr = addr;
    return listen(&laddr, port);
}

bool AsyncUDP::listenMulticast(const IPAddress addr, uint16_t port, uint8_t ttl)
{
    ip_addr_t laddr;
    laddr.addr = addr;
    return listenMulticast(&laddr, port, ttl);
}

bool AsyncUDP::connect(const IPAddress addr, uint16_t port)
{
    ip_addr_t daddr;
    daddr.addr = addr;
    return connect(&daddr, port);
}

size_t AsyncUDP::writeTo(const uint8_t *data, size_t len, const IPAddress addr, uint16_t port)
{
    ip_addr_t daddr;
    daddr.addr = addr;
    return writeTo(data, len, &daddr, port);
}

IPAddress AsyncUDP::listenIP()
{
    if(!_pcb){
        return IPAddress();
    }
    return IPAddress(_pcb->remote_ip.addr);
}

size_t AsyncUDP::write(const uint8_t *data, size_t len)
{
    return writeTo(data, len, &(_pcb->remote_ip), _pcb->remote_port);
}

size_t AsyncUDP::write(uint8_t data)
{
    return write(&data, 1);
}

size_t AsyncUDP::broadcastTo(uint8_t *data, size_t len, uint16_t port)
{
    return writeTo(data, len, IP_ADDR_BROADCAST, port);
}

size_t AsyncUDP::broadcastTo(const char * data, uint16_t port)
{
    return broadcastTo((uint8_t *)data, strlen(data), port);
}

size_t AsyncUDP::broadcast(uint8_t *data, size_t len)
{
    if(_pcb->local_port != 0) {
        return broadcastTo(data, len, _pcb->local_port);
    }
    return 0;
}

size_t AsyncUDP::broadcast(const char * data)
{
    return broadcast((uint8_t *)data, strlen(data));
}


size_t AsyncUDP::sendTo(AsyncUDPMessage &message, const ip_addr_t *addr, uint16_t port)
{
    if(!message) {
        return 0;
    }
    return writeTo(message.data(), message.length(), addr, port);
}

size_t AsyncUDP::sendTo(AsyncUDPMessage &message, const IPAddress addr, uint16_t port)
{
    if(!message) {
        return 0;
    }
    return writeTo(message.data(), message.length(), addr, port);
}

size_t AsyncUDP::send(AsyncUDPMessage &message)
{
    if(!message) {
        return 0;
    }
    return writeTo(message.data(), message.length(), &(_pcb->remote_ip), _pcb->remote_port);
}

size_t AsyncUDP::broadcastTo(AsyncUDPMessage &message, uint16_t port)
{
    if(!message) {
        return 0;
    }
    return broadcastTo(message.data(), message.length(), port);
}

size_t AsyncUDP::broadcast(AsyncUDPMessage &message)
{
    if(!message) {
        return 0;
    }
    return broadcast(message.data(), message.length());
}

AsyncUDP::operator bool()
{
    return _connected;
}

bool AsyncUDP::connected()
{
    return _connected;
}

esp_err_t AsyncUDP::lastErr() {
	return _lastErr;
}

void AsyncUDP::onPacket(AuPacketHandlerFunctionWithArg cb, void * arg)
{
    onPacket(std::bind(cb, arg, std::placeholders::_1));
}

void AsyncUDP::onPacket(AuPacketHandlerFunction cb)
{
    _handler = cb;
}

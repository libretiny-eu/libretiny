#include "server_drv.h"

#ifdef __cplusplus
extern "C" {
#include "ard_socket.h"
#include "platform_stdlib.h"
}
#endif

// Start server TCP on port specified
int ServerDrv::startServer(uint16_t port, uint8_t protMode)
{
    int sock;

	sock = start_server(port, protMode);
    if (sock >= 0) {
    	if(protMode == TCP_MODE)
    		sock_listen(sock, 1);
    }

    return sock;
}

int ServerDrv::getAvailable(int sock)
{
	return get_available(sock);
}

int ServerDrv::availData(int sock)
{
	int ret;
    uint8_t c;

	if (sock < 0)		
		return 0;
	
	if(_available) {
		return 1;
	} else {
        // flag = MSG_PEEK
		ret = get_receive(sock, &c, 1, 1, &_peer_addr, &_peer_port);
		if ( ret == 1 ) {
			_available = true;
			return 1;	
		} 	
		else{
			return ret;	
		}
	}
}

bool ServerDrv::getData(int sock, uint8_t *data, uint8_t peek)
{
	int ret = 0;
	int flag = 0;

    if (peek) {
        flag |= 1;
    } else {
        _available = false;
    }

	ret = get_receive(sock, data, 1, flag, &_peer_addr, &_peer_port);
	
	if (ret == 1) {
		return true;
	}
 
	return false;
}

int ServerDrv::getDataBuf(int sock, uint8_t *_data, uint16_t _dataLen)
{
	int ret;

    _available = false;

	ret = get_receive(sock, _data, _dataLen, 0, &_peer_addr, &_peer_port);

	return ret;
}

int ServerDrv::getLastErrno(int sock)
{
    return get_sock_errno(sock);
}

void ServerDrv::stopClient(int sock)
{
	stop_socket(sock);
    _available = false;
}

bool ServerDrv::sendData(int sock, const uint8_t *data, uint16_t len)
{
    int ret;

    if (sock < 0)
        return false;        	

    ret = send_data(sock, data, len);

    if (ret <= 0) {  
        return false;
    }

    return true;
}

bool ServerDrv::sendtoData(int sock, const uint8_t *data, uint16_t len, uint32_t peer_ip, uint16_t peer_port)
{
    int ret;

    if (sock < 0)
        return false;        	

    ret = sendto_data(sock, data, len, peer_ip, peer_port);

    if (ret == 0) {  
        return false;
    }

    return true;
}

int ServerDrv::startClient(uint32_t ipAddress, uint16_t port, uint8_t protMode)
{
    int sock;

    sock = start_client(ipAddress, port, protMode);

    return sock;
}

void ServerDrv::getRemoteData(int sock, uint32_t *ip, uint16_t *port)
{
    // TODO: These data may be outdated?
    *ip = _peer_addr;
    *port = _peer_port;
}

int ServerDrv::setSockRecvTimeout(int sock, int timeout)
{
    return set_sock_recv_timeout(sock, timeout);
}


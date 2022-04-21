#ifdef __cplusplus
extern "C" {
#endif

#include <platform_opts.h>
#include <lwip/sockets.h> 
#include <lwip/netif.h>
#include <platform/platform_stdlib.h>
#include "ard_socket.h"

int start_server(uint16_t port, uint8_t protMode)
{
    int _sock;
    int timeout;

    if(protMode == 0) {
        timeout = 3000;
        _sock = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        lwip_setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    } else {
        timeout = 1000;
        _sock = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        lwip_setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    }

    if (_sock < 0) {
        printf("\r\nERROR opening socket\r\n");
        return -1;
    }

    struct sockaddr_in localHost;
    memset(&localHost, 0, sizeof(localHost));

    localHost.sin_family = AF_INET;
    localHost.sin_port = htons(port);
    localHost.sin_addr.s_addr = INADDR_ANY;

    if (lwip_bind(_sock, (struct sockaddr *)&localHost, sizeof(localHost)) < 0) {
        printf("\r\nERROR on binding\r\n");
        return -1;
    }

    return _sock;
}

int sock_listen(int sock, int max)
{
	if(lwip_listen(sock , max) < 0){
		printf("\r\nERROR on listening\r\n");
		return -1;
	}	
	return 0;            
}

int get_available(int sock)
{
    int enable = 1;
    int timeout;
    int client_fd;
    int err;
	struct sockaddr_in cli_addr;

	socklen_t client = sizeof(cli_addr);

    do {
        client_fd = lwip_accept(sock, (struct sockaddr *) &cli_addr, &client);
        if (client_fd < 0) {
            err = get_sock_errno(sock);
            if (err != EAGAIN) {
                break;
            }
        }
    } while (client_fd < 0);

	if(client_fd  < 0){
        printf("\r\nERROR on accept\r\n");
		return -1;
	}
	else {
        timeout = 3000;
        lwip_setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
		timeout = 30000;
        lwip_setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        lwip_setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
        lwip_setsockopt(client_fd, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable));
		printf("\r\nA client connected to this server :\r\n[PORT]: %d\r\n[IP]:%s\r\n\r\n", ntohs(cli_addr.sin_port), inet_ntoa(cli_addr.sin_addr.s_addr));
		return client_fd;	
	}
}

int get_receive(int sock, uint8_t* data, int length, int flag, uint32_t *peer_addr, uint16_t *peer_port)
{
    int ret = 0;
    struct sockaddr from;
    socklen_t fromlen;

    uint8_t backup_recvtimeout = 0;
    int backup_recv_timeout, recv_timeout, len;

    if (flag & 0x01) {
        // for MSG_PEEK, we try to peek packets by changing receiving timeout to 10ms
        ret = lwip_getsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &backup_recv_timeout, &len);
        if (ret >= 0) {
            recv_timeout = 10;
            ret = lwip_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, sizeof(recv_timeout));
            if (ret >= 0) {
                backup_recvtimeout = 1;
            }
        }
    }

    ret = lwip_recvfrom(sock, data, length, flag, &from, &fromlen);
    if ( ret >= 0 ) {
        if (peer_addr != NULL) {
            *peer_addr = ((struct sockaddr_in *)&from)->sin_addr.s_addr;
        }
        if (peer_port != NULL) {
            *peer_port = ntohs(((struct sockaddr_in *)&from)->sin_port);
        }
    }

    if ((flag & 0x01) && (backup_recvtimeout == 1)) {
        // restore receiving timeout
        lwip_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &backup_recv_timeout, sizeof(recv_timeout));
    }

    return ret;
}

int get_sock_errno(int sock)
{
	int so_error;
	socklen_t len = sizeof(so_error);
	getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);
    return so_error;
}

int set_sock_recv_timeout(int sock, int timeout)
{
    return lwip_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

void stop_socket(int sock)
{
    lwip_close(sock);
}

int send_data(int sock, const uint8_t *data, uint16_t len)
{
    int ret;

    ret = lwip_write(sock, data, len);

    return ret;
}

int sendto_data(int sock, const uint8_t *data, uint16_t len, uint32_t peer_ip, uint16_t peer_port)
{
    int ret;

    struct sockaddr_in peer_addr;
    memset(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_addr.s_addr = peer_ip;
    peer_addr.sin_port = htons(peer_port);

    ret = lwip_sendto(sock, data, len, 0, (struct sockaddr*)&peer_addr, sizeof(struct sockaddr_in));

    return ret;
}

int start_client(uint32_t ipAddress, uint16_t port, uint8_t protMode)
{
    int enable = 1;
    int timeout;
    int _sock;

    if(protMode == 0)//tcp
        _sock = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    else
        _sock = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (_sock < 0) {
        printf("\r\nERROR opening socket\r\n");
        return -1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = ipAddress;
    serv_addr.sin_port = htons(port);

    if (protMode == 0){//TCP MODE
        if(connect(_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0){
            printf("\r\nConnect to Server successful!\r\n");

            timeout = 3000;
            lwip_setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
			timeout = 30000;
            lwip_setsockopt(_sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
            lwip_setsockopt(_sock, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
            lwip_setsockopt(_sock, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable));

            return _sock;
        }
        else{
            printf("\r\nConnect to Server failed!\r\n");
            stop_socket(_sock);
            return -1;
        }
    }
    else {
        //printf("\r\nUdp client setup Server's information successful!\r\n");
    }

    return _sock;
}

#ifdef __cplusplus
}
#endif

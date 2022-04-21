#ifndef ARD_SOCKET_H
#define ARD_SOCKET_H
#include "main.h"

int start_server(uint16_t port, uint8_t protMode);

int sock_listen(int sock, int max);

int get_available(int sock);

int get_receive(int sock, uint8_t* data, int length, int flag, uint32_t *peer_addr, uint16_t *peer_port);

int get_sock_errno(int sock);

int set_sock_recv_timeout(int sock, int timeout);

void stop_socket(int sock);

int send_data(int sock, const uint8_t *data, uint16_t len);

int sendto_data(int sock, const uint8_t *data, uint16_t len, uint32_t peer_ip, uint16_t peer_port);

int start_client(uint32_t ipAddress, uint16_t port, uint8_t protMode);

#endif

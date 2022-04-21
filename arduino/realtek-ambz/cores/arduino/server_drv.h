/*
  server_drv.h - Library for Arduino Wifi shield.
  Copyright (c) 2011-2014 Arduino.  All right reserved.

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

#ifndef Server_Drv_h
#define Server_Drv_h

#include <inttypes.h>

typedef enum eProtMode {TCP_MODE, UDP_MODE}tProtMode;
#define	DATA_LENTH		128
class ServerDrv
{
public:
	int startServer(uint16_t port, uint8_t protMode=TCP_MODE);
	int getAvailable(int sock);
    int startClient(uint32_t ipAddress, uint16_t port, uint8_t protMode=TCP_MODE);
    int getLastErrno(int sock);
	void stopClient(int sock);
	bool getData(int sock, uint8_t *data, uint8_t peek = 0);
	int getDataBuf(int sock, uint8_t *_data, uint16_t _dataLen);

    /* Usually used by TCP */
	bool sendData(int sock, const uint8_t *data, uint16_t len);

    /* Usually used by UDP */
	bool sendtoData(int sock, const uint8_t *data, uint16_t len, uint32_t peer_ip, uint16_t peer_port);

	int availData(int sock);
	
	void getRemoteData(int sock, uint32_t *ip, uint16_t *port);

    int setSockRecvTimeout(int sock, int timeout);

private:    	
	bool _available;

	uint32_t _peer_addr;
	uint16_t _peer_port;
};

#endif

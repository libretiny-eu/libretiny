/*
 * WebSocketClient + SSL/TSL
 * RTL8710AF pvvx 12/12/2016
 *
 */
#ifndef WEBSOCKETCLIENT_H_
#define WEBSOCKETCLIENT_H_

#include <Arduino.h>
#include <Stream.h>
extern "C" {
#include "libwsclient.h"
#include "wsclient_api.h"
}
  
class WebSocketClient {
public:
	WebSocketClient();
	~WebSocketClient();
	
/*************************************************************************************************
** Function Name  : begin
** Description    : Creating the websocket client context structure
** Input          : url:websocket server's url
**					port:websocket server's port, if not given, default 80 for "ws", 443 for "wss"
**					origin: the address or url of your self
** Return         : Ok: 1
**					Failed:  0
**************************************************************************************************/
	WebSocketClient(char *url, int port, char *path = NULL, char* origin = NULL);
	int begin(char *url, int port, char *path = NULL, char* origin = NULL);

/*************************************************************************************************
** Function Name  : connect
** Description    : Connecting to the websocket server
** Input          : wsclient: the websocket client context created by create_wsclientfunction
** Return         : Connected: the socket value
**					Failed:    -1
**************************************************************************************************/
	int connect();

/*************************************************************************************************
** Function Name  : ws_send
** Description    : Create the sending string data and copy to tx_buf
** Input          : message: the string that send to server(cannot exceeding the MAX_DATA_LEN
**					message_len: the length of the string
**					use_mask: 0/1; 1 means using mask for bynary
**					wsclient: the websocket client context
** Return         : None
**************************************************************************************************/
	void send(char* message, int message_len, int use_mask);

/*************************************************************************************************
** Function Name  : sendBinary
** Description    : Create the sending binary data and copy to tx_buf
** Input          : message: the binary that send to server(cannot exceeding the MAX_DATA_LEN
**					message_len: the length of the binary
**					use_mask: 0/1; 1 means using mask for bynary
**					wsclient: the websocket client context
** Return         : None
**************************************************************************************************/
	void sendBinary(uint8_t* message, int message_len, int use_mask);

/*************************************************************************************************
** Function Name  : sendPing
** Description    : Sending Ping to websocket server
** Input          : wsclient: the websocket client context
** Return         : None
**************************************************************************************************/
	void sendPing();

/*************************************************************************************************
** Function Name  : poll
** Description    : Receicing data from server and send the data in tx_buf
** Input          : timeout(in milliseconds)
					wsclient: the websocket client context
** Return         : None
**************************************************************************************************/
	void poll(int timeout);

/*************************************************************************************************
** Function Name  : dispatch
** Description    : callback function when getting message from server
** Input          : function that resolve the message received and the message length
** Return         : None
**************************************************************************************************/
	void dispatch(void (*callback)(wsclient_context *, int));

/*************************************************************************************************
** Function Name  : getReadyState
** Description    : Getting the connection status
** Input          : wsclient: the websocket client context
** Return         : readyStateValues(4 types: CLOSING, CLOSED, CONNECTING, OPEN ) 
**************************************************************************************************/
	readyStateValues getReadyState();

/*************************************************************************************************
** Function Name  : close
** Description    : Closing the connection with websocket server
** Input          : wsclient: the websocket client context
** Return         : None
**************************************************************************************************/
	void close();

/*************************************************************************************************
** Function Name  : ssl_func_on
** Description    : Set SSL/TSL function
** Input          : None
** Return         : None
**************************************************************************************************/
	void ssl_func_on(void);
	
private:
    wsclient_context *client;
};

#endif // WEBSOCKETCLIENT_H_
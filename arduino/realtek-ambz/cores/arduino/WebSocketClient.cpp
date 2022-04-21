/*
 * WebSocketClient + SSL/TSL
 * RTL8710AF pvvx 12/12/2016
 *
 */

#include "Arduino.h"
#include "WebSocketClient.h"

WebSocketClient::WebSocketClient() {
}

WebSocketClient::WebSocketClient(char *url, int port, char *path, char* origin) {
	client = create_wsclient(url, port, path, origin);
}

WebSocketClient::~WebSocketClient() {
	close();
}

int WebSocketClient::begin(char *url, int port, char *path, char* origin) {
	client = create_wsclient(url, port, path, origin);
	if(client != NULL) return 1;
	else return 0;
}

int WebSocketClient::connect() {
	if(client != NULL) return ws_connect_url(client);
	else return -1;
}

void WebSocketClient::send(char* message, int message_len, int use_mask) {
	if(client != NULL && client->readyState > CLOSED) {
		ws_send(message, message_len, use_mask, client);
		ws_poll(0, client);
	}
}

void WebSocketClient::sendBinary(uint8_t* message, int message_len, int use_mask) {
 	if(client != NULL && client->readyState > CLOSED) {
 		ws_sendBinary(message, message_len, use_mask, client);
		ws_poll(0, client);
	}
}

void WebSocketClient::sendPing() {
	if(client != NULL && client->readyState > CLOSED) {
		ws_sendPing(client);
		ws_poll(0, client);
	}
}

void WebSocketClient::poll(int timeout) {
	if(client != NULL) ws_poll(timeout, client);
}

readyStateValues WebSocketClient::getReadyState() {
	if(client != NULL) return ws_getReadyState(client);
	else return CLOSED;
}

void WebSocketClient::dispatch(void (*callback)(wsclient_context *, int))
{
	if(client != NULL) ws_dispatch(callback);
}

void WebSocketClient::close() {
	if(client != NULL) {
		ws_close(client);
	    if(client->ssl) {
	    	free(client->ssl);
	    	client->ssl = NULL;
	    }
		client = NULL;
	}
}

extern "C" void set_ssl_func(wsclient_context *wsclient); // in example_wsclient.c

void WebSocketClient::ssl_func_on(void)
{
  set_ssl_func(client);
/*  
  client->fun_ops.ssl_fun_ops.memory_set_own = memory_set_own;
  client->fun_ops.ssl_fun_ops.net_connect = net_connect;
  client->fun_ops.ssl_fun_ops.ssl_init = ssl_init;
  client->fun_ops.ssl_fun_ops.ssl_set_endpoint = ssl_set_endpoint;
  client->fun_ops.ssl_fun_ops.ssl_set_authmode = ssl_set_authmode;
  client->fun_ops.ssl_fun_ops.ssl_set_rng = ssl_set_rng;
  client->fun_ops.ssl_fun_ops.ssl_set_bio = ssl_set_bio;
  client->fun_ops.ssl_fun_ops.ssl_handshake = ssl_handshake;
  client->fun_ops.ssl_fun_ops.net_close = net_close;
  client->fun_ops.ssl_fun_ops.ssl_free = ssl_free;
  client->fun_ops.ssl_fun_ops.ssl_read = ssl_read;
  client->fun_ops.ssl_fun_ops.ssl_write = ssl_write;
  client->fun_ops.ssl_fun_ops.net_recv = net_recv;
  client->fun_ops.ssl_fun_ops.net_send = net_send;
  client->ssl = (void *)zalloc(sizeof(struct _ssl_context));  // 380 bytes
*/  
}

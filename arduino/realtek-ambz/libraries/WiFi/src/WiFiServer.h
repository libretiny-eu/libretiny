#ifndef wifiserver_h
#define wifiserver_h

#include "Arduino.h"
#include "api/Server.h"
#include "server_drv.h"

class WiFiClient;

class WiFiServer : public Server {
    private:
        uint16_t _port;
        int _sock_ser;
        ServerDrv serverfd;

    public:
        WiFiServer(uint16_t);

        WiFiClient available(uint8_t* status = NULL);

        void begin();
        virtual size_t write(uint8_t b);
        virtual size_t write(const uint8_t *buf, size_t size);
        //uint8_t status();

        using Print::write;
};

#endif

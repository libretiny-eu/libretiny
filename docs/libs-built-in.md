# Built-in libraries
(in alphabetical order)

## base64
- [Source](https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/base64.cpp): ESP32 Arduino Core
- [Reference](../ltapi/classbase64.md)

Helper base64 encoder used in some libs taken from ESP32.

## HTTPClient
- [Source](https://github.com/espressif/arduino-esp32/tree/master/libraries/HTTPClient): ESP32 Arduino Core
- [Reference](../ltapi/class_h_t_t_p_client.md)
- [Examples](https://github.com/espressif/arduino-esp32/tree/master/libraries/HTTPClient/examples)

HTTP(S) client.

## NetUtils

Utilities and common classes related to network.

- [ssl/MbedTLSClient.cpp](../ltapi/class_mbed_t_l_s_client.md) ([source](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFiClientSecure/src): ESP32 WiFiClientSecure)
- [IPv6Address.cpp](../ltapi/classarduino_1_1_i_pv6_address.md) ([source](https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/IPv6Address.cpp): ESP32 IPv6Address)
- [LwIPRxBuffer.cpp](../ltapi/class_lw_i_p_rx_buffer.md) ([source](https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFiClient.cpp): ESP32 WiFiClient)

## WebServer
- [Source](https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer/src): ESP32 Arduino Core
- [Reference](../ltapi/class_web_server.md)
- Examples:
    - [HelloServer](https://github.com/espressif/arduino-esp32/blob/master/libraries/WebServer/examples/HelloServer/HelloServer.ino)
    - [MultiHomedServers](https://github.com/espressif/arduino-esp32/blob/master/libraries/WebServer/examples/MultiHomedServers/MultiHomedServers.ino)

## WiFiMulti
- [Source](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi/src): ESP32 Arduino Core
- [Reference](../ltapi/class_wi_fi_multi.md)
- [Docs](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html#wifimulti)
- Examples:
    - [WiFiMulti](https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiMulti/WiFiMulti.ino)
    - [WiFiClientBasic](https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiClientBasic/WiFiClientBasic.ino)

Class for selecting best available AP from a list of several ones.

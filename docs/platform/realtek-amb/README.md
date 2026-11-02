# Realtek Ameba

The logic behind naming of Realtek chips and their series took me some time to figure out:

- RTL8xxxA - Ameba1/Ameba Series
- RTL8xxxB - [AmebaZ Series](../realtek-ambz/README.md)
- RTL8xxxC - [AmebaZ2/ZII Series](../realtek-ambz2/README.md)
- RTL8xxxCS - Ameba CS Series
- RTL8xxxD - AmebaD Series
- [Realtek product pages](https://www.realtek.com/en/products/communications-network-ics/category/802-11b-g-n)

As such, there are numerous CPUs with the same numbers but different series. Different Ameba series are not compatible with each other, which makes them require different code and SDKs.

<figure markdown>

  [![Realtek Ameba Series table](https://media.discordapp.net/attachments/983843871320580096/986231890879066172/unknown.png){ height="500" }](https://media.discordapp.net/attachments/983843871320580096/986231890879066172/unknown.png)
  <figcaption>Ameba series comparison table</figcaption>
</figure>

## Table from [www.e-paper-display.com](https://www.e-paper-display.com/products_detail/productId%3D529.html)

(modified a bit)

Realtek P/N | Series    | Protocol       | CPU    | Frequency | SRAM    | UART | SPI | I2S | ADC/DAC
------------|-----------|----------------|--------|-----------|---------| -----|-----|-----|--------
RTL8710BN   | Ameba Z   | WiFi           | ARM M4 | 125M      | 256KB   | 1    | 1   | N   | Y/N
RTL8710BX   | Ameba Z   | WiFi           | ARM M4 | 62.5M     | 256KB   | 1    | 1   | N   | Y/N
RTL8710CX   | Ameba Zii | WiFi           | ARM M4 | 100M      | 256KB   | 3    | 1   | N   | N/N
RTL8710CM   | Ameba Zii | WiFi           | ARM M4 | 100M      | 256K+4M | 3    | 1   | N   | N/N
RTL8720CN   | Ameba Zii | WiFi+BLE4.2    | ARM M4 | 100M      | 256KB   | 2    | 1   | N   | N/N
RTL8720CF   | Ameba Zii | WiFi+BLE4.2    | ARM M4 | 100M      | 256KB   | 2    | 1   | N   | N/N
RTL8720CM   | Ameba Zii | WiFi+BLE4.2    | ARM M4 | 100M      | 256KB   | 2    | 1   | N   | N/N
RTL8720CS   | Ameba CS  | WiFi+BLE5 Mesh | M0+M4  | 200M      | 512KB   | 2    | 2   | Y   | Y/Y
RTL8721CSM  | Ameba CS  | WiFi+BLE5 Mesh | M0+M4  | 200M      | 512K+4M | 3    | 2   | Y   | Y/Y
RTL8722CSM  | Ameba CS  | WiFi+BLE5 Mesh | M0+M4  | 200M      | 512K+4M | 4    | 2   | Y   | Y/Y
RTL8720DN   | Ameba D   | 2.4G+5G+BLE5   | M0+M4  | 200M      | 512K    | 2    | 1   | Y   | Y/Y
RTL8721DM   | Ameba D   | 2.4G+5G+BLE5   | M0+M4  | 200M      | 512K+4M | 3    | 2   | Y   | Y/Y
RTL8722DM   | Ameba D   | 2.4G+5G+BLE5   | M0+M4  | 200M      | 512K+4M | 4    | 2   | Y   | Y/Y

## Other chips

- [RTL8195AM](https://www.realtek.com/en/products/communications-network-ics/item/rtl8195am)
- RTL8710AF (found in amb1_arduino)
- [RTL8711AM](https://www.realtek.com/en/products/communications-network-ics/item/rtl8711am)
- [RTL8710BN](https://www.realtek.com/en/products/communications-network-ics/item/rtl8710bn)
- RTL8710BX (found in Tuya product pages)
- RTL8710B? (found in amb1_sdk)
- RTL8711B? (found in amb1_sdk)
- [RTL8710CM](https://www.realtek.com/en/products/communications-network-ics/item/rtl8710cm)
- RTL8722CSM (found in ambd_arduino)
- RTL8720DN (found in ambd_arduino)
- [RTL8721DM](https://www.realtek.com/en/products/communications-network-ics/item/rtl8721dm)
- RTL8722DM (found in ambd_arduino)

## MAC address

The station MAC address comes from the chip's eFuse (logical map offset `0x11A`). Chips that were never given a MAC there end up with the driver's built-in default - on AmebaZ2 that is `00:e0:4c:b7:23:00`. Every such device on your network then has the same MAC (and, through DHCP, the same IP address).

The BL3357 module in Broadlink RM4 devices is one of them. Its stock firmware keeps the real MAC in a flash sector at `0x4000` (it starts with the text `CALD`, the MAC follows at byte 8) and gives it to the driver itself. On AmebaZ2, LibreTiny does the same: if the eFuse holds no MAC and that sector is found, its address is used. Nothing is written anywhere, the address is only handed to the driver at WiFi start.

!!! warning
    The `0x4000` sector is where LibreTiny's own bootloader lives. Flashing a full LibreTiny image over the stock firmware **erases the MAC**, and the device falls back to the default address. Back up the stock flash first (`ltchiptool flash read`), the MAC is in that dump: bytes 8 to 13 of the sector at `0x4000`.

To set a MAC yourself (for example the one from such a backup), call `WiFi.setMacAddress()` **before** WiFi is started:

```cpp
void setup() {
	const uint8_t mac[6] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55};
	WiFi.setMacAddress(mac);
	WiFi.begin(ssid, password);
}
```

- The address is only kept in RAM and has to be set again after every reset. Nothing is written to the chip.
- It takes precedence over the MAC from eFuse or flash.
- The call is rejected while WiFi is running, and for all-zero, all-ones and multicast addresses.
- Use a locally administered address (second-lowest bit of the first byte set, e.g. `02:...`) unless the address is one you own.

!!! warning
    Do not call the SDK function `wifi_set_mac_address()` yourself. It programs the one-time-programmable eFuse: the change is permanent and the number of writes is limited. LibreTiny used to call it from `WiFi.setMacAddress()`, with the wrong argument format on top.

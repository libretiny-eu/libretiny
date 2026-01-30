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

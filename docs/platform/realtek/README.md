# Realtek Ameba - notes

The logic behind naming of Realtek chips and their series took me some time to figure out:

- RTL8xxxA - Ameba1/Ameba Series
- RTL8xxxB - AmebaZ Series
- RTL8xxxC - AmebaZ2/ZII Series
- RTL8xxxD - AmebaD Series

As such, there are numerous CPUs with the same numbers but different series, which makes them require different code and SDKs.

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
- and probably many more

Different Ameba series are not compatible with each other. Apparently, there isn't an official public SDK for AmebaZ that can support C++ properly.

# Dumping - Beken 72xx

- [rtltool.py](https://github.com/libretuya/ltchiptool/blob/master/ltchiptool/soc/ambz/util/rtltool.py)

## Dumping firmware

Dumping is also done using UART (TX2/RX2; Log_UART).

- Connect power to the board: GND and 3V3 (from your UART adapter or a higher-powered source).
- Connect an USB<->UART adapter to the chip: RX->TX1, TX->RX1.
- Put the chip into download mode: reset the chip, while pulling UART2_TX to GND. If successful, the chip should print few garbage characters every second.
- Disconnect TX from GND after powering up the board.
- Install Python.
- Grab `rtltool.py` from the link above.
- Run dumping: `python rtltool.py -p COM60 rf 0 0x200000 dump.bin`. Change the port (`-p`) to your adapter's port.

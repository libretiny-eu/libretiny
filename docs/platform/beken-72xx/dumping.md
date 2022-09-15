# Dumping - Beken 72xx

- [bk7231tools](https://github.com/notkmhn/bk7231tools)

## Dumping firmware

Dumping is also done using UART (TX1/RX1).

- Connect power to the board: GND and 3V3 (from your UART adapter or a higher-powered source).
- Connect an USB<->UART adapter to the chip: RX->TX1, TX->RX1.
- Install Python and Git.
- Clone bk7231tools: `git clone https://github.com/notkmhn/bk7231tools`
- `cd bk7231tools`
- Run dumping: `python bk7231tools.py read_flash -d COM96 -b 230400 -s 0 -c 512 dump.bin`. Change the port (`-p`) and baudrate (`-b`) if you want.

	!!! note
		The `--no-verify-checksum` is not required on BK7231N in latest versions of `bk7231tools`, despite the readme saying otherwise.

		It's advised to run without that option, to make sure the dump is valid.

- You have 10 seconds to reset the chip (pull CEN to GND, or power-cycle the board) after running the command. The program will then begin dumping.

	!!! note
		If you need to, you can increase the linking timeout using `--timeout` parameter.

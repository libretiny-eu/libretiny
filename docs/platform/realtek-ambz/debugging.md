# Debugging

Debugging of Realtek Ameba chips is possible and was tested with OpenOCD running remotely on a Raspberry Pi.

*(the following is applicable to Arduino framework, and was not tested with SDK framework)*

LibreTiny has ready-to-use OpenOCD config files:

- [platform/realtek-ambz/openocd/amebaz.cfg](../../../cores/realtek-ambz/misc/amebaz.cfg)

## Local debugger

It should be possible to use PlatformIO's built-in debugging capabilities directly, when plugging an OpenOCD-compatible debugger into your PC. As there are no debugger interfaces built into these IoT boards, you need to specify your interface of choice in `platformio.ini`:
```ini
[env:my_board]
openocd_interface = <interface name>
```
where `<interface name>` is for example `raspberrypi2-native`, `stlink`, etc.

## Remote debugger

Using a Raspberry Pi is probably the easiest option (and cheapest, as everyone has a spare Pi laying around).

Connect your Realtek board to your Pi, as per [Programming Microcontrollers using OpenOCD on a Raspberry Pi](https://learn.adafruit.com/programming-microcontrollers-using-openocd-on-raspberry-pi).

Check out [RPi BCM2711 GPIOs](https://elinux.org/RPi_BCM2711_GPIOs) to read more about BCM pin mappings.

TL;DR: Install OpenOCD. Conenct A14 to BCM GPIO#11, A15 to BCM GPIO#25. Remember to join GND together. Refer to [boards/](../../../boards/) for pinouts.

!!! note
	On Raspberry Pi 4, additional config might be needed:
	```
	bcm2835gpio peripheral_base 0xFE000000
	bcm2835gpio speed_coeffs 236181 60
	```
	Save the lines to a .cfg file, and pass it to OpenOCD using `-f file.cfg`.
	Read more [here](https://forums.raspberrypi.com/viewtopic.php?p=1541631&sid=915e8d039f24301b85c060318a44b7f0#p1541631).

Start OpenOCD like this (you also need your config file in the working directory):
```console
sudo openocd -f interface/raspberrypi2-native.cfg -f amebaz.cfg -c "bindto 0.0.0.0"
```
The bindto line is important, as it will allow remote connections.

Configure `platformio.ini` not to start local OpenOCD:
```ini
[env:my_board]
debug_tool = custom
debug_port = 192.168.0.33:3333
debug_server =
```
Replace IP with your Pi's address.

Done, go to PlatformIO in VSCode (or whatever you're using) and click `Start Debugging`.

## OpenOCD output

OpenOCD should show this if everything is connected properly:
```
alpine:~$ sudo openocd -f interface/raspberrypi2-native.cfg -f amebaz.cfg -c "bindto 0.0.0.0"
Open On-Chip Debugger 0.11.0
Licensed under GNU GPL v2
For bug reports, read
        http://openocd.org/doc/doxygen/bugs.html
BCM2835 GPIO nums: swclk = 11, swdio = 25

Warn : Interface already configured, ignoring
Info : Listening on port 6666 for tcl connections
Info : Listening on port 4444 for telnet connections
Info : BCM2835 GPIO JTAG/SWD bitbang driver
Info : clock speed 1001 kHz
Info : SWD DPIDR 0x2ba01477
Info : rtl8711b.cpu: hardware has 6 breakpoints, 4 watchpoints
Info : starting gdb server for rtl8711b.cpu on 3333
Info : Listening on port 3333 for gdb connections
```

## Auto reset

PlatformIO will reset your board by default when starting debugging. Sometimes this may not be desired. Also the current config is a bit buggy: VSCode thinks the board is halt, but it's actually running so you need to press continue for that first time.

To disable auto reset before and after debugging:
```ini
[env:my_board]
debug_init_cmds =
  target extended-remote $DEBUG_PORT ; remove this line if you're debugging locally
  $INIT_BREAK
;   monitor reset halt
  $LOAD_CMDS
  monitor init
;   monitor reset halt
```

## Technical details

GDB is first configured with `mem 0x8000000 0x8200000 ro` in order to mark flash memory as read-only. This makes GDB use hardware breakpoints, as software BPs don't work on these boards.

## More powerful playground

```cmd
Microsoft Windows [Version 6.1.7601]

Kuba@KUBA-PC C:\Users\Kuba
# telnet 192.168.0.33 4444
Open On-Chip Debugger
> mdw 0x8000000
0x08000000: 96969999

> halt
target halted due to debug-request, current mode: Thread
xPSR: 0x61000000 pc: 0x0000b462 msp: 0x1003ef5c
> reg
===== arm v7m registers
(0) r0 (/32): 0x035a9584
(1) r1 (/32): 0x00000015
[...]
===== Cortex-M DWT registers

> resume
>
```

## Useful OpenOCD commands

Run them in your power playground.

### Soft reset
Doesn't even disconnect from WiFi (which confuses the code and makes it disconnect anyway).
```
mww 0xE000ED0C 0x05FA0007
```

### UART upload mode
```
mww 0x40000138 0x8
mww 0xE000ED0C 0x05FA0007
```

### Hard Fault
```
halt
reg pc 0
resume
```

### ROM dump
```
> dump_image rom2.bin 0x0 0x80000
dumped 524288 bytes in 14.041406s (36.464 KiB/s)
```

### Flash dump
```
> dump_image flash.bin 0x8000000 0x200000
dumped 2097152 bytes in 54.447296s (37.614 KiB/s)
```

### Efuse dump
(or something that looks like it)
```
> dump_image efuse.bin 0x40000000 0x400
dumped 1024 bytes in 0.026813s (37.295 KiB/s)
```

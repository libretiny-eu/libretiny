# Debugging

Debugging of Lightning LN882x chips is possible and was tested with a JLink probe on OpenOCD and JLinkGDBServer.

*(the following is applicable to Arduino framework, and was not tested with SDK framework)*

LibreTiny has ready-to-use OpenOCD config files:

- [platform/lightning-ln882x/openocd/ln882x.cfg](../../../cores/lightning-ln882x/misc/ln882x.cfg)

## Local debugger

It should be possible to use PlatformIO's built-in debugging capabilities directly, when plugging an OpenOCD-compatible debugger into your PC. As there are no debugger interfaces built into these IoT boards, you need to specify your interface of choice in `platformio.ini`:

```ini
[env:my_board]
openocd_interface = <interface name>
```
where `<interface name>` is for example `raspberrypi2-native`, `stlink`, etc.

## JLink configuration

```ini
[env:my_board]
board = ln-02
debug_tool = custom
debug_server =
    ../JLink_Linux_V698e_x86_64/JLinkGDBServerExe
    -singlerun
    -if
    SWD
    -select
    USB
    -port
    3333
    -device
    Cortex-M4
debug_init_cmds =
    define pio_reset_halt_target
        monitor reset
    end
    target extended-remote $DEBUG_PORT
    ;monitor speed auto
    monitor reset
    $LOAD_CMDS
    $INIT_BREAK
upload_protocol = custom
upload_flags =
    -auto
    -startapp
    -exit
upload_command = ../JLink_Linux_V698e_x86_64/JFlashExe -openprj"../JLink_Linux_V698e_x86_64/ln882h.jflash" -open"${BUILD_DIR}/image_firmware.bin",0 $UPLOAD_FLAGS
debug_init_break = tbreak Reset_Handler
```

## Technical details

GDB is first configured with `mem 0x0000000 0x1000000 ro` in order to mark flash memory as read-only. This makes GDB use hardware breakpoints, as software BPs don't work on these boards.


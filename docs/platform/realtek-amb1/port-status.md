# Ameba1 runtime port status

Target: RTL8711AM, Cortex-M3, 256 KiB SRAM and 2 MiB external SDRAM.
No runtime or radio success is established by compilation.

## Builder contract

- Select `CONFIG_PLATFORM_8195A`, soft-float Cortex-M3 and the 8195a SDK libs.
- Force `_AppStart` from the core (`-Wl,--undefined=_AppStart`) before the SDK
  `_platform` archive. The SDK `InfraStart` calls `_AppStart`, not `APP_Start`.
  The core also supplies `ShowRamBuildInfo` so SDK `app_start.o` is not pulled in.
- Build SDK `component/common/api/wifi/wifi_conf.c`, `wifi_util.c`,
  `wifi_promisc.c`, and (if needed) the WPS/P2P/EAP configuration adapters.
  Build `wifi_ind.c` only for base builds: Arduino's WiFiEvents owns those symbols.
- Build SDK `component/common/drivers/wlan/realtek/src/osdep/lwip_intf.c` and
  `component/common/network/dhcp/dhcps.c`. Do not build SDK `lwip_netconf.c`:
  this core supplies a lwIP2 implementation.
- Arduino's shared `WiFiGeneric.cpp` must check `lt_amb1_lwip_ready()` after
  `LwIP_Init()` before setting its initialized flag or starting the radio.
  On failure restore diagnostic output and return false. Initialization is
  serialized and a partial failure is terminal until reboot, preventing a
  second tcpip thread or duplicate netif registration.
- Include SDK `component/common/network` globally for `<dhcp/dhcps.h>`.
- Use LibreTiny external lwIP's `amb1` port (ethernetif/sys_arch) and CM3
  FreeRTOS port. ESPHome task notifications need FreeRTOS 8.2.3 or newer.
- Package both SRAM and SDRAM load segments. Raw objcopy across their address
  gap is not a deployable image. The fridge loader needs authenticated image2
  packaging, not UF2 or AmebaZ XIP images.
- Include `.ARM.extab` and `.ARM.exidx` in the SDRAM load segment. The linker
  folds init/fini code and retained constructor arrays into the normal SDRAM
  text/rodata sections, and marks SRAM/SDRAM BSS NOLOAD.

Both the packaged and OpenBeken 8195a WLAN archives contain a stale
`rtl8711b_download_rsvd_page` reference in `rltk_stop_softap`. The linker maps
that name to the real archive-provided `rtl8195a_download_rsvd_page` function,
not a success stub or a function from the incompatible 8711b silicon library.

## Runtime and memory

`InfraStart` retains ownership of clocks, SDRAM and platform initialization.
The naked `_AppStart` trampoline aligns SP, clears SDRAM/TCM BSS, registers
FreeRTOS exception vectors, and calls `lt_main` for constructors/FAL/Arduino.
Ordinary text, rodata, data, BSS and RTOS heap reside in SDRAM. SRAM retains
image2 entry and explicitly tagged low-level sections. Tickless SDRAM suspend
is disabled pending hardware validation.

The factory MAC reader decodes the packet-encoded WLAN efuse logical map at
0x11A, including extended headers and later-word overrides. The 32-byte user
OTP API is not the WLAN map. Invalid/blank/multicast MACs are rejected with an
error; no fabricated device identity is substituted. A host mock test verified
decoder cases, but sticker agreement and driver identity remain hardware gates.
All three logical MAC words must be present; an incomplete efuse record cannot
silently substitute erased 0xFF bytes. DHCP status snapshots execute on the
tcpip thread rather than dereferencing a client pointer from the caller task.

## Serial mapping and limitations

- Logical Serial0 uses the mbed UART HAL (board PA7 TX / PA6 RX), interrupt RX.
- Logical Serial2 is the separate log UART (board PB0 TX / PB1 RX), not SDK
  peripheral UART2. It currently supports TX only and 8N1 framing; RX remains
  disabled rather than stealing the ROM console IRQ.
- Early printf always uses the log UART, never the appliance serial port.
- Software serial and unported peripheral APIs are not validated.
- DHCP setup is serialized through lwIP's tcpip netif API. The vendor WLAN
  glue still uses its own raw link-state calls, as in the existing ambz port.
  Association, DHCP, DNS and reconnect behavior require device testing.
- OTA capability depends on an authenticated image2 loader. The stock OTA
  family hooks are documented in `cores/common/base/api/lt_ota.h`.

## Storage policy

Flash is read-only in the default core. The qualification build option
`custom_amb1_flash_qualification` compiles the staged image2 adapter and
enables OTA; it is not a general-purpose write authorization.

The family FAL adapter returns byte counts on successful reads and `-1` on
failed reads, writes and erases (including zero-length write/erase requests). It
checks negative offsets, bounds, null buffers and overflow. RTL8195A stream
reads return 1 on success; that is converted to the requested FAL byte count.
Initialization does not call flash status setters or storage formatting.

`AddCoreSources` selects `base/port/*.c*` for both the amb parent and amb1;
matching filenames do NOT exclude the parent source. `BuildLibraries` prepends
each archive, putting the child before the parent. The family linker script's
`EXTERN(flash0)` forces the child device at archive scan time. Inspect the final
map: `flash0` and `lt_flash_obj` must originate in `fal_flash_amb1_port.c.o`,
never `fal_flash_ambz_port.c.o`. Do not force-load the inherited parent adapter.

### Required builder integration

The builder adds these flags to the amb1 family LINKFLAGS:

```
-Wl,--wrap=flash_write_word
-Wl,--wrap=flash_stream_write
-Wl,--wrap=flash_burst_write
-Wl,--wrap=flash_erase_sector
-Wl,--wrap=flash_erase_block
-Wl,--wrap=flash_erase_chip
-Wl,--wrap=flash_write_protect
-Wl,--wrap=flash_set_status
-Wl,--wrap=flash_reset_status
-Wl,--wrap=flash_set_extend_addr
```

The linker script asserts each wrapped symbol resolves to its guard: missing
flags fail linking rather than silently shipping a writable SDK path. SDK
integer mutators return 0 (SDK success is 1), not a fabricated success. Void
erase/protection APIs cannot report failure and log that limitation explicitly.
FAL returns `-1`, so LibreTiny's unsigned flash API reports zero written bytes.

### Boot/WLAN audit

- `lt_main` calls `lt_init_family`, variant init, banner, constructors, then
  `fal_init`; FAL root is assigned before the application starts. Family init
  only changes diagnostic globals and the UART selector, not flash contents.
- The SDK's `AP_SETTING_SECTOR=0xFE000`, `UART_SETTING_SECTOR=0xFC000`, and
  `FAST_RECONNECT_DATA=0x7F000` overlap native slot A on the WCATA007 board.
  They must not be used for provisioning.
- Packaged images are produced by `tools/amb1/uf2_pack.py`, which extracts both
  load segments from the linked ELF and validates the native image2 boundaries.

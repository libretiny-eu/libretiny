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
- OTA capability is disabled: the inherited AmebaZ XIP slot probes and boot
  flag writes are unsafe for the fridge's image2/HMAC loader. Public slot
  queries/switches return unavailable/failure without changing flash.

## First-bench storage policy

Flash is unconditionally read-only in this core. There is no writable opt-in
flag and no RAM-backed success substitute for persistence. The family FAL
adapter returns byte counts on successful reads and `-1` on failed reads,
writes and erases (including zero-length write/erase requests). It checks
negative offsets, bounds, null buffers and overflow. RTL8195A stream reads
return 1 on success; that is converted to the requested FAL byte count.
Initialization does not call flash status setters or storage formatting.

`AddCoreSources` selects `base/port/*.c*` for both the amb parent and amb1;
matching filenames do NOT exclude the parent source. `BuildLibraries` prepends
each archive, putting the child before the parent. The family linker script's
`EXTERN(flash0)` forces the child device at archive scan time. Inspect the final
map: `flash0` and `lt_flash_obj` must originate in `fal_flash_amb1_port.c.o`,
never `fal_flash_ambz_port.c.o`. Do not force-load the inherited parent adapter.

### Required builder integration

The builder owner must add these flags to the amb1 family LINKFLAGS (this core
deliberately does not modify shared builder files):

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

### Layout and preferences

The WCATA007 native image starts are 0xB000 and 0x177000, with slot A ending
at 0x177000. Preserve OpenBeken config at 0x2E3000..0x2FEFFF inclusive.
The proposed `system = 0x2E3000 + 0x51D000` is not reviewed free space: it
includes that config and extends to the end of the 8 MiB device. Do not use it
as an ESPHome settings region or infer a free area from slot starts alone.

ESPHome LibreTiny preferences call
`fdb_kvdb_init(&db, "esphome", "kvs", NULL, NULL)`: the required FAL partition
name is **kvs**, not kv or nvs. FlashDB init can automatically erase/format
invalid sector headers. `flash_write_interval: never` only affects subsequent
sync scheduling, not this initialization. Keep `kvs` absent for the first bench
build: initialization must report an error, not persistence success. Even if a
partition is mistakenly supplied, the FAL guard rejects writes and erases.
Before enabling real persistence, independently allocate/review a sector-aligned
`kvs` area outside both complete image slots, loader metadata, calibration and
the saved OpenBeken settings; FlashDB KVDB needs at least two sectors. Check
the exact linked FlashDB version's geometry requirements before choosing size.

### Boot/WLAN audit and remaining gate

- `lt_main` calls `lt_init_family`, variant init, banner, constructors, then
  `fal_init`; FAL root is assigned before the application starts. Family init
  only changes diagnostic globals and the UART selector, not flash contents.
- The SDK's `AP_SETTING_SECTOR=0xFE000`, `UART_SETTING_SECTOR=0xFC000`, and
  `FAST_RECONNECT_DATA=0x7F000` overlap native slot A. They are not valid free
  storage for this board and must not be used for provisioning.
- The family config forces OTA, simple config, flash-backed fast connect,
  JD Smart and fast DHCP off. SDK wifi_conf's persistence callback path is
  conditional on fast connect/JD Smart. Ordinary autoreconnect is distinct
  from persistent fast connect and need not be disabled.
- SDK OTA helpers call flash APIs directly, bypassing FAL. SDK guards block
  those public mutators as defense in depth; the LT OTA API remains disabled.
- Read APIs may lazily initialize the SDK SPI controller. `InfraStart` and
  `SpicFlashInitRtl8195A` reside in vendor archives; FAL guards and public SDK
  wrappers do not intercept ROM calls, same-object internal calls, direct
  memory-mapped flash writes, or lower-level SPIC mutation calls. This source
  audit does NOT prove those opaque paths never write. Before any bench boot,
  independently inspect the final linked call paths/archives for autonomous
  calibration/status/erase/program operations. If that cannot be established,
  the no-unreviewed-writes bench gate remains CLOSED. No hardware write test
  or assertion of boot-time safety follows from compilation.

Host contract test (no SDK/hardware):

```
python3 cores/realtek-amb1/tests/test_flash_readonly.py
```

The test compiles the actual adapter/guards with mocks and checks read offsets,
byte counts, SDK errors, invalid ranges, denied mutations and diagnostics.
It does not establish vendor ROM behavior or final firmware archive selection.

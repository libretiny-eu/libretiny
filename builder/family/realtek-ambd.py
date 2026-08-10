# Copyright (c) Bl00d-B0b 2026-08-06.

from os.path import join

from platformio.platform.base import PlatformBase
from platformio.platform.board import PlatformBoardConfig
from SCons.Script import DefaultEnvironment, Environment

env: Environment = DefaultEnvironment()
platform: PlatformBase = env.PioPlatform()
board: PlatformBoardConfig = env.BoardConfig()
queue = env.AddLibraryQueue("realtek-ambd")
env.ConfigureFamily()

# Bench diagnostic: platformio.ini "custom_fault_dump = 1" installs the
# fault handler from cores/realtek-ambd/base/fixups/fault_dump.c. Project
# build_flags do not reach the core sources, so it has to be an env define.
if env.GetProjectOption("custom_fault_dump", "0") == "1":
    env.Append(CPPDEFINES=[("LT_AMBD_FAULT_DUMP", 1)])

# Serial numbers ports by the variant (SERIAL0 = LOGUART, SERIAL2 = LP UART), the
# printf port by the SDK (2 = LOGUART), so point Serial at the console explicitly.
env.Append(CPPDEFINES=[("LT_UART_DEFAULT_SERIAL", 0)])

COMPONENT_DIR = join("$SDK_DIR", "component")
SOC_DIR = join(COMPONENT_DIR, "soc", "realtek", "amebad")
FREERTOS_DIR = join(COMPONENT_DIR, "os", "freertos", "freertos_v10.2.0")
LWIP_DIR = join(COMPONENT_DIR, "common", "network", "lwip", "lwip_v2.2.0")
FREERTOS_PORT_DIR = join(
    FREERTOS_DIR, "Source", "portable", "GCC", "RTL8721D_HP", "non_secure"
)

# KM4 (Cortex-M33) application build. KM0 image and both boot images ship
# precompiled in the framework package; only the KM4 image2 is compiled here.
queue.AppendPublic(
    CCFLAGS=[
        "-march=armv8-m.main+dsp",
        "-mthumb",
        "-mcmse",
        "-mfloat-abi=hard",
        "-mfpu=fpv5-sp-d16",
    ],
    CPPDEFINES=[
        ("CONFIG_PLATFORM_8721D", 1),
        ("ARM_CORE_CM4", 1),
        ("CONFIG_BUILD_RAM", 1),
    ],
    LINKFLAGS=[
        "-mcpu=cortex-m33",
        "-mthumb",
        "-mcmse",
        "-mfloat-abi=hard",
        "-mfpu=fpv5-sp-d16",
        "--specs=nano.specs",
        # Boot ROM + Img2EntryFun0 do the startup; app_start.c owns _init.
        "-nostartfiles",
        "-Wl,--gc-sections",
        # Pull the image2 entry table object (nothing references it; the boot
        # ROM finds it by section address) — anchors app_start -> main.
        "-Wl,--undefined=Img2EntryFun0",
        # Newlib (scanned after the lib group) needs these from our syscalls.c;
        # -u loads them while the group is still open.
        "-Wl,-u,_exit",
        "-Wl,-u,_kill",
        "-Wl,-u,_getpid",
        # lt_init_family()/lt_init_variant() are declared weak, so a plain call
        # does not pull their archive members in — force them.
        "-Wl,-u,lt_init_family",
        "-Wl,-u,lt_init_variant",
        # The SDK's app_start() calls main() directly; wrap it to run
        # lt_main()'s startup first (fixups/lt_main_hook.c).
        "-Wl,-wrap,main",
    ],
    CPPPATH=[
        join(SOC_DIR, "cmsis"),
        join(SOC_DIR, "fwlib", "include"),
        join(SOC_DIR, "app", "monitor", "include"),
        join(SOC_DIR, "app", "xmodem"),
        join(SOC_DIR, "swlib", "include"),
        join(SOC_DIR, "swlib", "string"),
        join(SOC_DIR, "misc"),
        join(COMPONENT_DIR, "os", "freertos"),
        # SDK-bundled FreeRTOS 10.2.0 with the vendor RTL8721D_HP port; the
        # library-freertos/-port packages carry no CM33 port, so the SDK copy
        # is the one that builds (same choice the vendor KM4 project makes).
        join(FREERTOS_DIR, "Source", "include"),
        join(FREERTOS_PORT_DIR),
        join("$SDK_DIR", "project", "OpenBeken", "inc", "inc_hp"),
        join(COMPONENT_DIR, "os", "os_dep", "include"),
        join(COMPONENT_DIR, "common", "api"),
        join(COMPONENT_DIR, "common", "api", "wifi"),
        join(COMPONENT_DIR, "common", "api", "network", "include"),
        join(COMPONENT_DIR, "common", "api", "wifi", "rtw_wpa_supplicant", "src"),
        join(
            COMPONENT_DIR,
            "common",
            "api",
            "wifi",
            "rtw_wpa_supplicant",
            "wpa_supplicant",
        ),
        join(
            COMPONENT_DIR, "common", "api", "wifi", "rtw_wpa_supplicant", "src", "utils"
        ),
        join(
            COMPONENT_DIR, "common", "drivers", "wlan", "realtek", "wlan_ram_map", "rom"
        ),
        # rtl8721d_ota.h includes mbedtls/version.h; 2.4.0 is the SDK default
        # (sources join the build later with the SSL/OTA feature work).
        join(COMPONENT_DIR, "common", "network", "ssl", "mbedtls-2.4.0", "include"),
        join(COMPONENT_DIR, "common", "network", "ssl", "ssl_ram_map", "rom"),
        # mbed-style HAL the shared realtek-amb Arduino core builds on
        join(COMPONENT_DIR, "common", "mbed", "api"),
        join(COMPONENT_DIR, "common", "mbed", "hal"),
        join(COMPONENT_DIR, "common", "mbed", "hal_ext"),
        join(COMPONENT_DIR, "common", "mbed", "targets", "hal", "rtl8721d"),
        join(COMPONENT_DIR, "common", "api", "platform"),
        join(COMPONENT_DIR, "common", "network"),
        # SDK-bundled lwIP 2.2.0: ESPHome's socket layer needs lwip_readv,
        # which 2.0.2 does not provide.
        join(LWIP_DIR, "src", "include"),
        join(LWIP_DIR, "port", "realtek"),
        join(LWIP_DIR, "port", "realtek", "freertos"),
        join(COMPONENT_DIR, "common", "drivers", "wlan", "realtek", "include"),
        join(COMPONENT_DIR, "common", "drivers", "wlan", "realtek", "src", "osdep"),
    ],
)

# Sources: fwlib + app glue compile with the core; wlan/BT link from the
# prebuilt SDK archives (lib_wlan.a etc.) shipped in the framework package.
queue.AddLibrary(
    name="ambd_fwlib",
    base_dir=SOC_DIR,
    srcs=[
        "+<fwlib/ram_hp/*.c>",
        "+<fwlib/crypto/*.c>",
        "+<app/monitor/ram/*.c>",
        "+<fwlib/ram_common/*.c>",
        # KM0-only peripheral (its APBPeriph_QDEC ids exist only on the LP core)
        "-<fwlib/ram_common/rtl8721d_qdec.c>",
        "+<fwlib/usrcfg/rtl8721d_wificfg.c>",
        "+<fwlib/usrcfg/rtl8721d_bootcfg.c>",
        "+<fwlib/usrcfg/rtl8721d_ipccfg.c>",
        "+<fwlib/usrcfg/rtl8721dhp_intfcfg.c>",
        "+<fwlib/usrcfg/rtl8721dhp_boot_trustzonecfg.c>",
        "+<misc/*.c>",
        # HTTP/SD-card OTA app code (drags in fatfs); LibreTiny OTA is uf2ota
        "-<misc/rtl8721d_ota.c>",
    ],
    includes=[],
)

# OS glue: osdep service, heap config (defines psram_dev_config), cmsis_os.
queue.AddLibrary(
    name="ambd_osdep",
    base_dir=COMPONENT_DIR,
    srcs=[
        "+<os/os_dep/osdep_service.c>",
        "+<os/os_dep/device_lock.c>",
        "+<os/os_dep/psram_reserve.c>",
        "+<os/freertos/cmsis_os.c>",
        "+<os/freertos/freertos_service.c>",
        "+<os/freertos/freertos_backtrace_ext.c>",
        "+<common/mbed/targets/hal/rtl8721d/*.c>",
        # WiFi API + wlan driver OS glue (wext_wlan_indicate, promisc,
        # rltk_wlan_set_netif_info, lwip_intf) and the DHCP server
        "+<common/api/wifi/wifi_conf.c>",
        # wifi_ind.c is NOT built: WiFiEvents.cpp provides wifi_indication
        "+<common/api/wifi/wifi_promisc.c>",
        "+<common/api/wifi/wifi_util.c>",
        "+<common/api/wifi/rtw_wpa_supplicant/wpa_supplicant/wifi_eap_config.c>",
        "+<common/api/wifi/rtw_wpa_supplicant/wpa_supplicant/wifi_p2p_config.c>",
        "+<common/api/wifi/rtw_wpa_supplicant/wpa_supplicant/wifi_wps_config.c>",
        "+<common/api/lwip_netconf.c>",
        "+<common/drivers/wlan/realtek/src/osdep/lwip_intf.c>",
        "+<common/network/dhcp/dhcps.c>",
        "+<common/network/ssl/ssl_ram_map/ssl_ram_map.c>",
        # wifi_conf.c installs mbedTLS allocators through this
        "+<common/network/ssl/mbedtls-2.4.0/library/platform.c>",
    ],
    includes=[],
    options=dict(CFLAGS=["-w"]),
)

# Compiled separately: needs the full SoC surface (psram_dev_config) that its
# own include chain does not pull — the vendor build force-includes it too.
queue.AddLibrary(
    name="ambd_osheap",
    base_dir=COMPONENT_DIR,
    srcs=["+<os/freertos/freertos_heap5_config.c>"],
    includes=[],
    options=dict(CFLAGS=["-w", "-include", "ameba_soc.h"]),
)

# SDK-bundled lwIP 2.2.0 with the Realtek port (netif_rx and friends).
queue.AddLibrary(
    name="ambd_lwip",
    base_dir=LWIP_DIR,
    srcs=[
        "+<src/api/*.c>",
        "+<src/core/*.c>",
        "+<src/core/ipv4/*.c>",
        "+<src/netif/ethernet.c>",
        # SNTP client (ESPHome's sntp component links against it)
        "+<src/apps/sntp/sntp.c>",
        # mDNS responder (ESPHome's mdns component)
        "+<src/apps/mdns/*.c>",
        "+<port/realtek/freertos/*.c>",
        "+<port/realtek/*.c>",
    ],
    includes=[],
    options=dict(CFLAGS=["-w"]),
)

# Prebuilt SDK archives (power management now; wlan/BT join with their features)
env.Append(
    LIBPATH=[
        join(
            "$SDK_DIR",
            "project",
            "OpenBeken",
            "GCC-RELEASE",
            "project_hp",
            "asdk",
            "lib",
            "application",
        )
    ],
    LIBS=["_pmc_hp", "_wlan", "_websocket", "_wps"],
)

# FreeRTOS from the SDK (vendor port + heap_5, per the KM4 project makefile).
queue.AddLibrary(
    name="ambd_freertos",
    base_dir=FREERTOS_DIR,
    srcs=[
        "+<Source/tasks.c>",
        "+<Source/list.c>",
        "+<Source/croutine.c>",
        "+<Source/queue.c>",
        "+<Source/timers.c>",
        "+<Source/event_groups.c>",
        "+<Source/stream_buffer.c>",
        "+<Source/portable/GCC/RTL8721D_HP/non_secure/port.c>",
        "+<Source/portable/GCC/RTL8721D_HP/non_secure/portasm.c>",
        "+<Source/portable/MemMang/heap_5.c>",
    ],
    includes=[],
    options=dict(CFLAGS=["-w"]),
)

# Generate the KM4 img2 linker script (template carries the vendor layout;
# the ROM symbol table and memory layout resolve via LIBPATH INCLUDEs).
env.GenerateLinkerScript(board, board.get("build.ldscript"))

queue.BuildLibraries()

# Image packaging: the KM4 ELF becomes the km0_km4_image2 in an OTA slot:
# [KM0 XIP][KM0 RAM][KM4 XIP][KM4 RAM][terminator], each with a 32-byte header.
# The KM0 half is a prebuilt blob; only the KM4 half is compiled here.
IMG2_SIGN = b"81958711"
KM4_XIP_ADDRESS = 0x0E000020
KM4_RAM_ADDRESS = 0x10005000
PSRAM_TERM_ADDRESS = 0x02000020
KM4_XIP_SECTIONS = [".xip_image2.text", ".ARM.exidx"]
KM4_RAM_SECTIONS = [".ram_image2.entry", ".ram_image2.text", ".ram_image2.data"]


def section_header(length, address):
    import struct

    return IMG2_SIGN + struct.pack("<II", length, address) + b"\xff" * 16


def build_ota_image(target, source, env):
    from os.path import join as pjoin
    from subprocess import run

    elf = str(source[0])
    build_dir = env.subst("$BUILD_DIR")
    objcopy = env.subst("$OBJCOPY")

    def extract(sections, name):
        out = pjoin(build_dir, name)
        cmd = [objcopy, "-O", "binary"]
        for s in sections:
            cmd += ["-j", s]
        run(cmd + [elf, out], check=True)
        with open(out, "rb") as f:
            return f.read()

    km0_path = env.subst(pjoin("$FAMILY_DIR", "misc", "km0_image2_all.bin"))
    with open(km0_path, "rb") as f:
        km0 = f.read()

    km4_xip = extract(KM4_XIP_SECTIONS, "km4_xip.bin")
    km4_ram = extract(KM4_RAM_SECTIONS, "km4_ram.bin")

    image = km0
    # XIP payloads are executed in place, so keep them page-congruent with the
    # slot offset (both OTA offsets are 4K-aligned).
    image += b"\xff" * ((-len(image)) % 0x1000)
    image += section_header(len(km4_xip), KM4_XIP_ADDRESS) + km4_xip
    image += section_header(len(km4_ram), KM4_RAM_ADDRESS) + km4_ram
    image += section_header(0, PSRAM_TERM_ADDRESS)

    with open(str(target[0]), "wb") as f:
        f.write(image)


image_ota = "${BUILD_DIR}/image_ota.${FLASH_OTA1_OFFSET}.bin"
env.Command(
    image_ota,
    "${BUILD_DIR}/${PROGNAME}.elf",
    env.VerboseAction(build_ota_image, "Packing $TARGET"),
)
env.Depends("${BUILD_DIR}/firmware.uf2", image_ota)
env.Replace(
    UF2OTA=[
        f"{image_ota},{image_ota}=device:ota1,ota2;flasher:ota1,ota2",
    ],
)

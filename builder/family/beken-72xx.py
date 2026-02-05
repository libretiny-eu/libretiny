# Copyright (c) Kuba Szczodrzyński 2022-06-13.

from os.path import join

import click
from platformio.platform.base import PlatformBase
from platformio.platform.board import PlatformBoardConfig
from SCons.Script import DefaultEnvironment, Environment

env: Environment = DefaultEnvironment()
board: PlatformBoardConfig = env.BoardConfig()
platform: PlatformBase = env.PioPlatform()
queue = env.AddLibraryQueue("beken-72xx")
env.ConfigureFamily()

ROOT_DIR = join("$SDK_DIR", "beken378")
APP_DIR = join(ROOT_DIR, "app")
DRIVER_DIR = join(ROOT_DIR, "driver")
FUNC_DIR = join(ROOT_DIR, "func")

# Load sys_config.h into env
env.LoadConfig(join("$FAMILY_DIR", "base", "config", "sys_config.h"))

# Check used version of BDK
bdk_version = platform.versions["framework-beken-bdk"]
if not bdk_version.startswith("3.0."):
    click.secho(
        "Only 3.0.x versions of 'framework-beken-bdk' are supported "
        f"(found '{bdk_version}'). Please modify or remove the custom "
        "version specification.",
        fg="red",
    )
    exit(1)

# Supported SoC types
SOC_BK7231 = 1
SOC_BK7231U = 2
SOC_BK7251 = 3  # also: BK7221U, BK7252
SOC_BK7271 = 4
SOC_BK7231N = 5
SOC_BK7236 = 6
SOC_BK7238 = 7
SOC_BK7252N = 8  # also: BK7253
# Library names for each SoC
SOC_NAMES = {
    SOC_BK7231: "bk7231",
    SOC_BK7231U: "bk7231u",
    SOC_BK7251: "bk7251",
    SOC_BK7271: "bk7271",
    SOC_BK7231N: "bk7231n",
    SOC_BK7236: "bk7236",
    SOC_BK7238: "bk7238",
    SOC_BK7252N: "bk7252n",
}
# Minimum BDK version for each SoC
SOC_BDK = {
    SOC_BK7231: None,
    SOC_BK7231U: (3, 0, 21),
    SOC_BK7251: (3, 0, 11),
    SOC_BK7271: (3, 1, 5),
    SOC_BK7231N: (3, 0, 4),
    SOC_BK7236: None,
    SOC_BK7238: (3, 0, 56),
    SOC_BK7252N: (3, 0, 76),
}

# Supported BLE versions
BLE_VERSION_4_2 = 1
BLE_VERSION_5_1 = 2
BLE_VERSION_5_2 = 3
# BLE source code directory names
BLE_VERSIONS = {
    BLE_VERSION_4_2: "ble_4_2",
    BLE_VERSION_5_1: "ble_5_1",
    BLE_VERSION_5_2: "ble_5_2",
}

# Define constants used for choosing sources
SOC = env.Cfg("CFG_SOC_NAME")
BLE = env.Cfg("CFG_SUPPORT_BLE") and env.Cfg("CFG_BLE_VERSION")
BDK = tuple(map(int, bdk_version.split(".")))

# Validate SoC support in BDK
if not SOC_BDK[SOC]:
    click.secho(f"Selected chip '{SOC_NAMES[SOC]}' is not supported in BDK.", fg="red")
    exit(1)
if BDK < SOC_BDK[SOC]:
    click.secho(
        f"Selected chip '{SOC_NAMES[SOC]}' is not supported in BDK {bdk_version}. "
        f"The minimum supported version is 3.0.{SOC_BDK[SOC] % 100}.",
        fg="red",
    )
    exit(1)

# Flags
queue.AppendPublic(
    CCFLAGS=[
        "-mcpu=arm968e-s",
        "-march=armv5te",
        "-mthumb",
        "-mthumb-interwork",
        "-Wno-write-strings",
        "-Wno-attributes",
        # anything higher, like -O2 or -Os, causes random issues
        # like bootlooping, missing (blank) strings, random lockups during boot
        "+<-O1>",
        "-<-Os>",
    ],
    CPPDEFINES=[
        # SDK options
        ("CFG_OS_FREERTOS", "1"),
        ("MBEDTLS_CONFIG_FILE", r"\"tls_config.h\""),
        ("WIFI_BLE_COEXIST", "1"),
        ("WOLFSSL_BEKEN", env.Cfg("CFG_WPA3")),
        "MBEDTLS_KEY_EXCHANGE__SOME__PSK_ENABLED",
        ("INCLUDE_xTaskGetHandle", "1"),
        # Provide BDK version to code
        ("CFG_BDK_VERSION", f"{BDK[0]}{BDK[1]:02d}{BDK[2]:02d}"),
    ],
    ASFLAGS=[
        "-mcpu=arm968e-s",
        "-march=armv5te",
        "-marm",
        "-mthumb-interwork",
        "-g",
        "-x",
        "assembler-with-cpp",
    ],
    LINKFLAGS=[
        "-mcpu=arm968e-s",
        "-marm",
        "-mthumb-interwork",
        "--specs=nano.specs",
        "-Wl,-wrap,bk_flash_get_info",
        "-Wl,-wrap,bk_flash_erase",
        "-Wl,-wrap,bk_flash_write",
        "-Wl,-wrap,bk_flash_read",
        # stdio wrappers (base/port/printf.c)
        "-Wl,-wrap,bk_printf",
    ],
)
queue.AppendPrivate(
    CCFLAGS=[
        "-Wno-comment",
        "-Wno-char-subscripts",
        "-Wno-missing-braces",
        "-Wno-return-type",
    ],
    CFLAGS=[
        "-Wno-format",
        "-Wno-unknown-pragmas",
    ],
)

srcs_core = []

# Fix for BK7231T's bootloader compatibility
if board.get("build.bkboot_version") in ["1.0.5-bk7231s", "bk7231q"]:
    # this has to be public, so that fixups/intc.c sees it
    queue.AppendPublic(CPPDEFINES=[("CFG_SUPPORT_BOOTLOADER", "1")])
    queue.AddLibrary(
        name="bdk_boot",
        base_dir="$CORES_DIR/beken-72xx/base/fixups",
        srcs=["+<boot_handlers_bk7231u.S>"],
    )
else:
    srcs_core.append("+<driver/entry/boot_handlers.S>")

# Sources - from framework-beken-bdk/beken378/beken_src.mk
queue.AddLibrary(
    name="bdk_core",
    base_dir=ROOT_DIR,
    srcs=[
        "+<app/app.c>",
        "+<app/config/param_config.c>",
        "+<driver/entry/boot_vectors.S>",
        "+<func/wlan_ui/wlan_ui.c>",
        *srcs_core,
    ],
    includes=[
        "+<app>",
        "+<app/config>",
        "+<common>",
        "+<driver/entry>",
        "+<driver/intc>",
        "+<release>",
        "+<../release>",
    ],
)

# Sources - app module
queue.AddLibrary(
    name="bdk_app",
    base_dir=APP_DIR,
    srcs=[
        "+<http/*.c>",
        "+<net_work/*.c>",
        "+<standalone-ap/*.c>",
        "+<standalone-station/*.c>",
        "+<video_work/*.c>",
    ],
    includes=[
        "+<http>",
        "+<net_work>",
        "+<standalone-ap>",
        "+<standalone-station>",
        "+<video_work>",
    ],
)

# Sources - drivers
queue.AddLibrary(
    name="bdk_driver",
    base_dir=DRIVER_DIR,
    srcs=[
        "+<driver.c>",
        "+<common/*.c>",
        "+<dma/*.c>",
        "+<fft/*.c>",
        "+<flash/*.c>",
        "+<gpio/*.c>",
        "+<i2c/i2c2.c>",
        "+<icu/*.c>",
        "+<macphy_bypass/*.c>",
        "+<phy/*.c>",
        "+<pwm/*.c>",
        "-<pwm/pwm_bk7271.c>",
        "-<pwm/pwm_new.c>",
        "+<rw_pub/*.c>",
        "+<saradc/*.c>",
        "+<security/*.c>",
        "+<spi/*.c>",
        "-<spi/spi_psram.c>",
        "+<spidma/*.c>",
        "+<sys_ctrl/*.c>",
        "+<uart/uart.c>",
        "+<wdt/*.c>",
        # BK7252N-specific
        SOC == SOC_BK7252N and "+<general_dma/general_dma_bk7252n.c>",
        SOC == SOC_BK7252N and "+<i2c/i2c1_bk7252n.c>",
        SOC == SOC_BK7252N and "+<i2s/i2s_bk7252n.c>",
        SOC == SOC_BK7252N and "+<irda/irda_bk7252n.c>",
        SOC == SOC_BK7252N and "+<jpeg/jpeg.c>",
        SOC == SOC_BK7252N and "+<qspi/qspi_bk7252n.c>",
        SOC == SOC_BK7252N and "+<rtc/rtc_reg.c>",
        # All other SoCs
        SOC != SOC_BK7252N and "+<calendar/calendar.c>",
        SOC != SOC_BK7252N and "+<general_dma/general_dma.c>",
        SOC != SOC_BK7252N and "+<i2c/i2c1.c>",
        SOC != SOC_BK7252N and "+<i2s/i2s.c>",
        SOC != SOC_BK7252N and "+<irda/irda.c>",
        SOC != SOC_BK7252N and "+<jpeg/jpeg_encoder.c>",
        SOC != SOC_BK7252N and "+<qspi/qspi.c>",
    ],
    includes=[
        "+<common>",
        "+<common/reg>",
        "+<dma>",
        "+<flash>",
        "+<general_dma>",
        "+<gpio>",
        "+<i2c>",
        "+<icu>",
        "+<include>",
        "+<jpeg>",
        "+<phy>",
        "+<pwm>",
        "+<rc_beken>",
        "+<rw_pub>",
        "+<spi>",
        "+<spidma>",
        "+<sys_ctrl>",
        "+<uart>",
        "+<usb>",
        "+<../ip/**>",
    ],
    options=dict(CCFLAGS=["-Wno-unused-variable"]),
)

# Sources - functional components
queue.AddLibrary(
    name="bdk_func",
    base_dir=FUNC_DIR,
    srcs=[
        "+<func.c>",
        "+<airkiss/*.c>",
        "+<base64/*.c>",
        "+<bk7011_cal/*.c>",
        SOC != SOC_BK7231 and "+<ble_wifi_exchange/*.c>",
        "+<camera_intf/*.c>",
        "+<force_sleep/*.c>",
        "+<hostapd_intf/*.c>",
        "+<joint_up/*.c>",
        "+<lwip_intf/dhcpd/*.c>",
        "+<misc/*.c>",
        "-<misc/fake_clock.c>",  # fixups
        "+<net_param_intf/*.c>",
        "+<ntp/*.c>",
        "+<power_save/*.c>",
        "-<power_save/low_voltage_compensation.c>",
        "-<power_save/low_voltage_ps.c>",
        "+<rtc/*.c>",
        "-<rtc/alarm.c>",
        "+<rwnx_intf/*.c>",
        "+<saradc_intf/*.c>",
        "+<security/*.c>",
        "+<sim_uart/*.c>",
        "+<spidma_intf/*.c>",
        "+<temp_detect/*.c>",
        "+<usb_plug/*.c>",
        "+<user_driver/*.c>",
        "-<user_driver/BkDriverQspi.c>",
        "+<utf8/*.c>",
        "+<video_transfer/*.c>",
        "+<wpa_supplicant_2_9/src/crypto/crypto_mbedtls.c>",
    ],
    includes=[
        "+<base64>",
        "+<ble_wifi_exchange>",
        "+<camera_intf>",
        "+<ethernet_intf>",
        "+<force_sleep>",
        "+<include>",
        "+<joint_up>",
        "+<lwip_intf>",  # for config/lwipopts.h
        "+<misc>",
        "+<ntp>",
        "+<power_save>",
        "+<rf_test>",
        "+<rf_use>",
        "+<rtc>",
        "+<rwnx_intf>",
        "+<saradc_intf>",
        "+<sensor>",
        "+<spidma_intf>",
        "+<temp_detect>",
        "+<uart_debug>",
        "+<usb>",
        "+<user_driver>",
        "+<utf8>",
        "+<video_transfer>",
        "+<wpa_supplicant_2_9/bk_patch>",
        "+<wpa_supplicant_2_9/hostapd>",
        "+<wpa_supplicant_2_9/src>",
        "+<wpa_supplicant_2_9/src/ap>",
        "+<wpa_supplicant_2_9/src/common>",
        "+<wpa_supplicant_2_9/src/drivers>",
        "+<wpa_supplicant_2_9/src/utils>",
        "+<wpa_supplicant_2_9/src/wps>",
        "+<wpa_supplicant_2_9/wpa_supplicant>",
    ],
    options=dict(
        CPPDEFINES=[
            # only used in func.c
            ("SDK_COMMIT_ID", "NULL"),
        ]
    ),
)

# Sources - FreeRTOS
freertos_opts = dict(
    CCFLAGS=[
        # build FreeRTOS port in ARM mode
        "+<-marm>",
        "-<-mthumb>",
    ],
)
env.Replace(FREERTOS_PORT="beken-bdk", FREERTOS_PORT_DEFINE="BEKEN_BDK")
queue.AddExternalLibrary("freertos", options=freertos_opts)
queue.AddExternalLibrary("freertos-port", options=freertos_opts)
queue.AddLibrary(
    name="bdk_freertos_thumb",
    base_dir=ROOT_DIR,
    srcs=[
        "+<os/**/*.c>",
        "-<os/platform_stub.c>",
    ],
    includes=[
        "+<os/*>",
    ],
    options=dict(
        CPPDEFINES=[
            # Make FreeRTOS modifications visible
            ("FREERTOS_PORT_BEKEN_BDK", "1"),
        ]
    ),
)

# Sources - lwIP
queue.AddExternalLibrary("lwip", port="bdk")

# Sources - mbedTLS 2.6.0
queue.AddLibrary(
    name="bdk_mbedtls",
    base_dir=join(FUNC_DIR, "mbedtls"),
    srcs=[
        "+<mbedtls/library/*.c>",
        "+<mbedtls-port/src/*.c>",
    ],
    includes=[
        "+<mbedtls/include>",
        "+<mbedtls-port/inc>",
    ],
    options=dict(
        CCFLAGS=[
            "-Wno-unused-variable",
            "-Wno-implicit-function-declaration",
            "-w",
        ],
    ),
)

# Sources - wolfSSL (for WPA3)
# * BDK 3.0.56 added an option whether to use mbedTLS or wolfSSL for crypto
#   in wpa_supplicant, but it wasn't used yet because WPA3 was disabled.
# * BDK 3.0.62 enabled WPA3 on BK7231N/U/7238/7251, which added a dependency on
#   an external crypto library (wolfSSL by default).
# * BDK 3.0.70 switched BK7231N and BK7238 to mbedTLS.
use_wpa_wolfssl = False
if BDK >= (3, 0, 62) and SOC in (SOC_BK7231N, SOC_BK7231U, SOC_BK7238, SOC_BK7251):
    use_wpa_wolfssl = True
if BDK >= (3, 0, 70) and SOC in (SOC_BK7231N, SOC_BK7238):
    use_wpa_wolfssl = False
if env.Cfg("CFG_USE_MBEDTLS"):
    use_wpa_wolfssl = False
if use_wpa_wolfssl:
    queue.AddLibrary(
        name="bdk_wolfssl",
        base_dir=join(FUNC_DIR, "wolfssl"),
        srcs=[
            "+<wolfcrypt/src/ecc.c>",
            "+<wolfcrypt/src/hmac.c>",
            "+<wolfcrypt/src/md5.c>",
            "+<wolfcrypt/src/memory.c>",
            "+<wolfcrypt/src/random.c>",
            "+<wolfcrypt/src/sha.c>",
            "+<wolfcrypt/src/sha256.c>",
            "+<wolfcrypt/src/sha512.c>",
            "+<wolfcrypt/src/tfm.c>",
            "+<wolfcrypt/src/wolfmath.c>",
        ],
        includes=[
            "+<.>",
        ],
    )

# Sources - BLE version
if env.Cfg("CFG_SUPPORT_BLE"):
    platform = (
        "bk7238" if SOC == SOC_BK7238 else "bk7252n" if SOC == SOC_BK7252N else "7231n"
    )
    if BDK >= (3, 0, 56):
        ble_base_dir = join(DRIVER_DIR, "ble", BLE_VERSIONS[BLE])
    elif BLE == BLE_VERSION_4_2:
        ble_base_dir = join(DRIVER_DIR, "ble")
    elif BLE == BLE_VERSION_5_1:
        ble_base_dir = join(DRIVER_DIR, "ble_5_x_rw")
    queue.AddLibrary(
        name=f"bdk_{BLE_VERSIONS[BLE]}",
        base_dir=ble_base_dir,
        srcs=[
            # BLE 4.2 (BK7231U, BK7251, BK7271)
            BLE == BLE_VERSION_4_2 and "+<ble.c>",
            BLE == BLE_VERSION_4_2 and "+<modules/app/src/*.c>",
            BLE == BLE_VERSION_4_2 and "-<modules/app/src/app_mesh.c>",
            BLE == BLE_VERSION_4_2 and "+<plactform/**/*.c>",
            BLE == BLE_VERSION_4_2 and "+<profiles/*/src/*.c>",
            # BLE 5.x
            BLE != BLE_VERSION_4_2 and "+<ble_pub/app/src/*.c>",
            BLE != BLE_VERSION_4_2 and "+<ble_pub/ui/ble_ui.c>",
            # BLE 5.1 (BK7231N, BK7236)
            BLE == BLE_VERSION_5_1 and "+<ble_pub/prf/*.c>",
            BLE == BLE_VERSION_5_1 and "+<ble_pub/profiles/comm/src/*.c>",
            BLE == BLE_VERSION_5_1 and "+<ble_pub/profiles/sdp/src/*.c>",
            # BLE 5.2 (BK7238, BK7252N, BK7253)
            BLE == BLE_VERSION_5_2 and "+<ble_pub/profiles/bas/bass/src/bass.c>",
            BLE == BLE_VERSION_5_2 and "+<ble_pub/profiles/bk_comm/src/*.c>",
            BLE == BLE_VERSION_5_2 and "+<ble_pub/profiles/bk_sdp/src/sdp_common.c>",
            BLE == BLE_VERSION_5_2 and "+<ble_pub/profiles/dis/diss/src/diss.c>",
            BLE == BLE_VERSION_5_2 and "+<ble_pub/profiles/find/findt/src/findt.c>",
            BLE == BLE_VERSION_5_2 and "+<ble_pub/profiles/hogp/hogpd/src/hogpd.c>",
            BLE == BLE_VERSION_5_2 and "+<hci/controller_hci.c>",
            # SoC-specific (BLE 5.x)
            BLE != BLE_VERSION_4_2 and f"+<platform/{platform}/**/*.c>",
            "-<ble_pub/app/src/app_ble_task.c>",
            f"-<platform/{platform}/nvds/src/nvds.c>",
        ],
        includes=[
            # BLE 4.2
            BLE == BLE_VERSION_4_2 and "+<.>",
            BLE == BLE_VERSION_4_2 and "+<beken_ble_sdk/*/include>",
            BLE == BLE_VERSION_4_2 and "+<beken_ble_sdk/mesh/src/dbg>",
            BLE == BLE_VERSION_4_2 and "+<beken_ble_sdk/mesh/src/models/include>",
            BLE == BLE_VERSION_4_2 and "+<config>",
            BLE == BLE_VERSION_4_2 and "+<modules/**>",
            BLE == BLE_VERSION_4_2 and "+<plactform/arch>",
            BLE == BLE_VERSION_4_2 and "+<plactform/driver/*>",
            BLE == BLE_VERSION_4_2 and "+<plactform/include>",
            BLE == BLE_VERSION_4_2 and "+<plactform/modules/include>",
            BLE == BLE_VERSION_4_2 and "+<profiles/*/api>",
            BLE == BLE_VERSION_4_2 and "+<profiles/*/include>",
            # BLE 5.x
            BLE != BLE_VERSION_4_2 and "+<arch/**>",
            BLE != BLE_VERSION_4_2 and "+<ble_lib/ip/ble/**>",
            BLE != BLE_VERSION_4_2 and "+<ble_lib/ip/*/*>",
            BLE != BLE_VERSION_4_2 and "+<ble_lib/modules/*/*>",
            BLE != BLE_VERSION_4_2 and "+<ble_pub/app/api>",
            BLE != BLE_VERSION_4_2 and "+<ble_pub/ui>",
            # BLE 5.1
            BLE == BLE_VERSION_5_1 and "+<ble_pub/prf>",
            BLE == BLE_VERSION_5_1 and "+<ble_pub/profiles/*/api>",
            # BLE 5.2
            BLE == BLE_VERSION_5_2 and "+<ble_lib/modules/rwip/import/reg>",
            BLE == BLE_VERSION_5_2 and "+<ble_pub/profiles/bas/bass/api>",
            BLE == BLE_VERSION_5_2 and "+<ble_pub/profiles/bk_comm/api>",
            BLE == BLE_VERSION_5_2 and "+<ble_pub/profiles/bk_sdp/api>",
            BLE == BLE_VERSION_5_2 and "+<ble_pub/profiles/dis/diss/api>",
            BLE == BLE_VERSION_5_2 and "+<ble_pub/profiles/find/findt/api>",
            BLE == BLE_VERSION_5_2 and "+<ble_pub/profiles/hogp>",
            BLE == BLE_VERSION_5_2 and "+<ble_pub/profiles/hogp/hogpd/api>",
            # SoC-specific (BLE 5.x)
            BLE != BLE_VERSION_4_2 and f"+<platform/{platform}/config>",
            BLE != BLE_VERSION_4_2 and f"+<platform/{platform}/driver/*>",
            BLE != BLE_VERSION_4_2 and f"+<platform/{platform}/entry>",
            BLE != BLE_VERSION_4_2 and f"+<platform/{platform}/rwip/api>",
            BLE != BLE_VERSION_4_2 and f"+<platform/{platform}/rwip/import/reg>",
        ],
    )

# Libs & linker config
queue.AppendPublic(
    LIBPATH=[
        join("$SDK_DIR", "beken378", "lib"),
        join("$SDK_DIR", "beken378", "func", "airkiss"),
    ],
    LIBS=[
        "airkiss",
        "sensor",
        # "wpa", # this is compiled from func/hostapd_intf/hostapd_intf.c
        SOC != SOC_BK7231 and f"ble_{SOC_NAMES[SOC]}",
        SOC == SOC_BK7251 and "usb",
        f"cal_{SOC_NAMES[SOC]}",
        f"rf_test_{SOC_NAMES[SOC]}",
        f"rf_use_{SOC_NAMES[SOC]}",
        f"rwnx_{SOC_NAMES[SOC]}",
        f"supplicant_{SOC_NAMES[SOC]}",
        f"uart_debug_{SOC_NAMES[SOC]}",
        "gcc",
        "g",
        "c",
        "m",
        "nosys",
    ],
)

# Misc options
env.Replace(
    SIZEPROGREGEXP=r"^(?:\.vectors|\.text|\.rodata|\.data|\.ARM\.exidx)\s+([0-9]+).*",
    SIZEDATAREGEXP=r"^(?:\.vectors|\.data|\.bss)\s+([0-9]+).*",
    SIZECHECKCMD="$SIZETOOL -A -d $SOURCES",
    SIZEPRINTCMD="$SIZETOOL -B -d $SOURCES",
)
# Generate linker scripts with correct flash offsets
env.GenerateLinkerScript(board, board.get("build.ldscript"))


def to_offset(addr: int) -> int:
    return int(addr + (addr // 32) * 2)


# Calculate RBL header offset
app_offs = int(env["FLASH_APP_OFFSET"], 16)
app_size = int(board.get("build.bkrbl_size_app"), 16)
rbl_offs = to_offset(app_size) - 102
env.Replace(FLASH_RBL_OFFSET=f"0x{app_offs + rbl_offs:06X}")

# Build all libraries
queue.BuildLibraries()

# Rename Arduino's delay() to delayMilliseconds()
env.Append(
    CPPDEFINES=[
        ("'delay(ms)'", "'delayMilliseconds(ms)'"),
    ],
)

# Main firmware outputs and actions
image_app_crc = "${BUILD_DIR}/image_${MCULC}_app.${FLASH_APP_OFFSET}.crc"
image_app_rblh = "${BUILD_DIR}/image_${MCULC}_app.${FLASH_RBL_OFFSET}.rblh"
image_ota_rbl = "${BUILD_DIR}/image_${MCULC}_app.ota.rbl"
env.Replace(
    # linker command (encryption + packaging)
    LINK='${LTCHIPTOOL} link2bin ${BOARD_JSON} "" ""',
    # UF2OTA input list
    UF2OTA=[
        # app binary image (enc+crc) for flasher
        f"{image_app_crc}=flasher:app",
        # app RBL header (with crc) for flasher
        f"{image_app_rblh}+{rbl_offs}=flasher:app",
        # OTA RBL package for device only
        f"{image_ota_rbl}=device:download",
    ],
)

# Copyright (c) Kuba Szczodrzyński 2022-06-13.

from os.path import join

from platformio.platform.board import PlatformBoardConfig
from SCons.Script import DefaultEnvironment, Environment

env: Environment = DefaultEnvironment()
board: PlatformBoardConfig = env.BoardConfig()
queue = env.AddLibraryQueue("beken-72xx")
env.ConfigureFamily()

ROOT_DIR = join("$SDK_DIR", "beken378")
APP_DIR = join(ROOT_DIR, "app")
DRIVER_DIR = join(ROOT_DIR, "driver")
FUNC_DIR = join(ROOT_DIR, "func")

# Load sys_config.h into env
env.LoadConfig(join("$FAMILY_DIR", "base", "config", "sys_config.h"))

# Define vars used during build
SOC_BK7231 = 1
SOC_BK7231U = 2
SOC_BK7221U = 3
SOC_BK7251 = 3
SOC_BK7271 = 4
SOC_BK7231N = 5
SOC_BK7236 = 6
SOC_NAMES = {
    SOC_BK7231: "bk7231u",
    SOC_BK7231U: "bk7231u",
    SOC_BK7251: "bk7251",
    SOC_BK7271: "bk7271",
    SOC_BK7231N: "bk7231n",
    SOC_BK7236: "bk7236",
}
SOC = env.Cfg("CFG_SOC_NAME")
WPA_VERSION = "wpa_supplicant_2_9" if env.Cfg("CFG_USE_WPA_29") else "hostapd-2.5"

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
        "+<calendar/*.c>",
        "+<common/*.c>",
        "+<dma/*.c>",
        "+<fft/*.c>",
        "+<flash/*.c>",
        "+<general_dma/*.c>",
        "+<gpio/*.c>",
        "+<i2c/*.c>",
        "+<i2s/*.c>",
        "+<icu/*.c>",
        "+<irda/*.c>",
        "+<jpeg/*.c>",
        "+<macphy_bypass/*.c>",
        "+<phy/*.c>",
        "+<pwm/*.c>",
        "-<pwm/pwm_bk7271.c>",
        "-<pwm/pwm_new.c>",
        "+<qspi/*.c>",
        "+<rw_pub/*.c>",
        "+<saradc/*.c>",
        "+<security/*.c>",
        "+<spidma/*.c>",
        "+<sys_ctrl/*.c>",
        "+<uart/uart.c>",
        "+<wdt/*.c>",
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
        SOC != SOC_BK7231 and "+<ble_wifi_exchange/*.c>",
        "+<camera_intf/*.c>",
        "+<hostapd_intf/*.c>",
        "+<joint_up/*.c>",
        "+<lwip_intf/dhcpd/*.c>",
        "+<misc/*.c>",
        "-<misc/fake_clock.c>",  # fixups
        "+<net_param_intf/*.c>",
        "+<power_save/*.c>",
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
    ],
    includes=[
        "+<base64>",
        "+<ble_wifi_exchange>",
        "+<camera_intf>",
        "+<easy_flash>",
        "+<easy_flash/inc>",
        "+<easy_flash/port>",
        "+<ethernet_intf>",
        "+<include>",
        "+<joint_up>",
        "+<lwip_intf>",  # for config/lwipopts.h
        "+<power_save>",
        "+<rf_test>",
        "+<rf_use>",
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
        f"+<{WPA_VERSION}/bk_patch>",
        f"+<{WPA_VERSION}/hostapd>",
        f"+<{WPA_VERSION}/src>",
        f"+<{WPA_VERSION}/src/ap>",
        f"+<{WPA_VERSION}/src/common>",
        f"+<{WPA_VERSION}/src/drivers>",
        f"+<{WPA_VERSION}/src/utils>",
        f"+<{WPA_VERSION}/wpa_supplicant>",
    ],
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

# Sources - chip-specific drivers
if SOC in [SOC_BK7231, SOC_BK7231U, SOC_BK7251]:
    queue.AddLibrary(
        name="bdk_driver_spi",
        base_dir=join(DRIVER_DIR, "spi"),
        srcs=[
            "+<spi.c>",
            "+<spi_master.c>",
            "+<spi_slave.c>",
        ],
    )
if SOC in [SOC_BK7231N]:
    queue.AddLibrary(
        name="bdk_driver_spi",
        base_dir=join(DRIVER_DIR, "spi"),
        srcs=[
            "+<spi_bk7231n.c>",
            "+<spi_master_bk7231n.c>",
            "+<spi_slave_bk7231n.c>",
        ],
    )
if SOC in [SOC_BK7251]:
    queue.AddLibrary(
        name="bdk_bk7251",
        base_dir=ROOT_DIR,
        srcs=[
            "+<driver/audio/*.c>",
            "+<driver/sdcard/*.c>",
            "+<func/audio/*.c>",
            "+<func/sd_music/*.c>",
        ],
        includes=[
            "+<driver/audio>",
            "+<driver/sdcard>",
        ],
    )

# Sources - enabled through config
if env.Cfg("CFG_SDIO"):
    queue.AddLibrary(
        name="bdk_driver_sdio",
        base_dir=ROOT_DIR,
        srcs=[
            "+<driver/sdio/*.c>",
            "+<func/sdio_intf/*.c>",
        ],
    )
if env.Cfg("CFG_BK_AWARE"):
    queue.AddLibrary(
        name="bdk_driver_sdio",
        base_dir="$SDK_DIR",
        srcs=[
            "+<beken378/func/bk_aware/*.c>",
            "+<demos/wifi/bk_aware/*.c>",
        ],
        includes=[
            "+<beken378/func/bk_aware>",
        ],
    )
if env.Cfg("CFG_USE_SDCARD_HOST"):
    queue.AddLibrary(
        name="bdk_func_fatfs",
        base_dir=join(FUNC_DIR, "fatfs"),
        srcs=[
            "+<*.c>",
            "-<test_fatfs.c>",
        ],
        includes=[
            "+<.>",
        ],
    )
if env.Cfg("CFG_WPA3"):
    queue.AddLibrary(
        name="bdk_wolfssl",
        base_dir=join(FUNC_DIR, "wolfssl"),
        srcs=[
            "+<wolfcrypt/src/ecc.c>",
            "+<wolfcrypt/src/memory.c>",
            "+<wolfcrypt/src/random.c>",
            "+<wolfcrypt/src/sha256.c>",
            "+<wolfcrypt/src/tfm.c>",
            "+<wolfcrypt/src/wolfmath.c>",
        ],
        includes=[
            "+<.>",
        ],
    )
if env.Cfg("CFG_SUPPORT_BLE") and env.Cfg("CFG_BLE_VERSION") == env.Cfg(
    "BLE_VERSION_4_2"
):
    queue.AddLibrary(
        name="bdk_ble_4_2",
        base_dir=join(DRIVER_DIR, "ble"),
        srcs=[
            "+<**/*.c>",
        ],
        includes=[
            "+<.>",
            "+<**/include>",
            "+<beken_ble_sdk/mesh/src/dbg>",
            "+<config>",
            "+<modules/**>",
            "+<plactform/arch>",
            "+<plactform/driver/*>",
            "+<profiles/*/api>",
        ],
    )
if env.Cfg("CFG_SUPPORT_BLE") and env.Cfg("CFG_BLE_VERSION") == env.Cfg(
    "BLE_VERSION_5_x"
):
    queue.AddLibrary(
        name="bdk_ble_5_x",
        base_dir=join(DRIVER_DIR, "ble_5_x_rw"),
        srcs=[
            "+<**/*.c>",
            "-<ble_pub/app/src/app_ble_task.c>",
            "-<platform/7231n/nvds/src/nvds.c>",
        ],
        includes=[
            "+<**/api>",
            "+<arch/**>",
            "+<ble_lib/ip/ble/**>",
            "+<ble_lib/ip/hci/src>",
            "+<ble_lib/ip/sch/import>",
            "+<ble_lib/modules/*/src>",
            "+<ble_pub/prf>",
            "+<ble_pub/ui>",
            "+<platform/7231n/**>",
            "-<platform/7231n/nvds/**>",
        ],
    )
if env.Cfg("ATSVR_CFG"):
    queue.AddLibrary(
        name="bdk_atsvr",
        base_dir=join(FUNC_DIR, "at_server"),
        srcs=[
            "+<**/*.c>",
        ],
        includes=[
            "+<.>",
            "+<*>",
        ],
    )
if env.Cfg("CFG_USB") or env.Cfg("CFG_USE_SDCARD_HOST"):
    queue.AddLibrary(
        name="bdk_driver_usb",
        base_dir=ROOT_DIR,
        srcs=[
            "+<driver/usb/*.c>",
            "+<func/usb/*.c>",
        ],
        includes=[
            "+<driver/usb/**>",
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
        "usb",
        # "wpa", # this is compiled from func/hostapd_intf/hostapd_intf.c
        SOC != SOC_BK7231 and f"ble_{SOC_NAMES[SOC]}",
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

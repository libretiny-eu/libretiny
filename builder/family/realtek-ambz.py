# Copyright (c) Kuba Szczodrzyński 2022-04-20.

from os.path import join

from platformio.platform.board import PlatformBoardConfig
from SCons.Script import Builder, DefaultEnvironment, Environment

env: Environment = DefaultEnvironment()
board: PlatformBoardConfig = env.BoardConfig()
queue = env.AddLibraryQueue("realtek-ambz")
env.ConfigureFamily()

# Flags
queue.AppendPublic(
    CCFLAGS=[
        "-mcpu=cortex-m4",
        "-mthumb",
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16",
        "-fno-short-enums",
    ],
    CPPDEFINES=[
        # other options
        "M3",
        "CONFIG_PLATFORM_8711B",
        ("ERRNO", "1"),  # for LwIP
        "MBEDTLS_KEY_EXCHANGE__SOME__PSK_ENABLED",  # enable PSK in mbedTLS
        # "MBEDTLS_DEBUG_C",
        "MBED_PERIPHERALNAMES_H",  # see fixups/cmsis.h
    ],
    LINKFLAGS=[
        "-mcpu=cortex-m4",
        "-mthumb",
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16",
        "--specs=nano.specs",
        # the entrypoint in ROM (?)
        "-Wl,--entry=Reset_Handler",
        # start function table in startup.c
        "-Wl,--undefined=gImage2EntryFun0",
        "-Wl,-wrap,rom_psk_CalcGTK",
        "-Wl,-wrap,rom_psk_CalcPTK",
        "-Wl,-wrap,CalcMIC",
        "-Wl,-wrap,CheckMIC",
        "-Wl,-wrap,aes_80211_encrypt",
        "-Wl,-wrap,aes_80211_decrypt",
        "-Wl,-wrap,DecGTK",
        # ROM stdlib (wraps/stdlib.c)
        # stock SDK defines these as macros
        "-Wl,-wrap,atoi",
        "-Wl,-wrap,atol",
        "-Wl,-wrap,strtol",
        "-Wl,-wrap,strtoul",
        "-Wl,-wrap,rand",
        "-Wl,-wrap,strcat",
        "-Wl,-wrap,strcpy",
        "-Wl,-wrap,strncat",
        "-Wl,-wrap,strncpy",
        "-Wl,-wrap,strchr",
        "-Wl,-wrap,strcmp",
        "-Wl,-wrap,strlen",
        "-Wl,-wrap,strncmp",
        "-Wl,-wrap,strpbrk",
        "-Wl,-wrap,strstr",
        "-Wl,-wrap,strtok",
        "-Wl,-wrap,memchr",
        "-Wl,-wrap,memcmp",
        "-Wl,-wrap,memcpy",
        "-Wl,-wrap,memmove",
        "-Wl,-wrap,memset",
        "-Wl,-wrap,strsep",
        # stdio wrappers (base/port/printf.c)
        "-Wl,-wrap,rtl_printf",
        "-Wl,-wrap,rtl_sprintf",
        "-Wl,-wrap,rtl_snprintf",
        "-Wl,-wrap,rtl_vsnprintf",
        "-Wl,-wrap,rtl_vsnprintf_r",
        "-Wl,-wrap,rtl_vprintf",
        "-Wl,-wrap,rtl_vfprintf",
        "-Wl,-wrap,DiagPrintf",
        "-Wl,-wrap,DiagSPrintf",
        "-Wl,-wrap,DiagSnPrintf",
        "-Wl,-wrap,prvDiagPrintf",
        "-Wl,-wrap,prvDiagSPrintf",
        "-Wl,-wrap,VSprintf",
        "-Wl,-wrap,LOG_PRINTF",
        "-Wl,-wrap,__rtl_vfprintf_r_v1_00",
    ],
)
queue.AppendPrivate(
    CFLAGS=[
        "-Wno-implicit-function-declaration",
        "-Wno-incompatible-pointer-types",
        "-Wno-int-conversion",
        "-Wno-pointer-sign",
    ],
)

# Sources - from SDK project/realtek_amebaz_va0_example/GCC-RELEASE/application.mk
# - "console" is disabled as it introduces build error, and is generally useless
# - "utilities example" are also not really needed
queue.AddLibrary(
    name="ambz_sdk",
    base_dir="$SDK_DIR",
    srcs=[
        # NOTE: a fixup is used instead, to remove default main()
        # "+<component/soc/realtek/8711b/cmsis/device/app_start.c>",
        "+<component/soc/realtek/8711b/fwlib/ram_lib/startup.c>",
        "+<component/soc/realtek/8711b/cmsis/device/system_8195a.c>",
        "+<component/soc/realtek/8711b/app/monitor/ram/rtl_trace.c>",
        "+<component/common/api/wifi/rtw_wpa_supplicant/wpa_supplicant/wifi_eap_config.c>",
        "+<component/common/api/wifi/rtw_wpa_supplicant/wpa_supplicant/wifi_p2p_config.c>",
        "+<component/common/api/wifi/rtw_wpa_supplicant/wpa_supplicant/wifi_wps_config.c>",
        "+<component/common/api/wifi/wifi_conf.c>",
        "ARDUINO" not in "ENV" and "+<component/common/api/wifi/wifi_ind.c>",
        "+<component/common/api/wifi/wifi_promisc.c>",
        "+<component/common/api/wifi/wifi_simple_config.c>",
        "+<component/common/api/wifi/wifi_util.c>",
        "+<component/common/api/lwip_netconf.c>",
        "+<component/common/drivers/wlan/realtek/src/osdep/lwip_intf.c>",
        "+<component/common/network/dhcp/dhcps.c>",
        "+<component/common/network/ssl/ssl_ram_map/ssl_ram_map.c>",
        "+<component/os/freertos/cmsis_os.c>",
        "+<component/os/freertos/freertos_service.c>",
        "+<component/os/os_dep/device_lock.c>",
        "+<component/os/os_dep/osdep_service.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/analogin_api.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/dma_api.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/efuse_api.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/flash_api.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/gpio_api.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/gpio_irq_api.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/i2c_api.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/i2s_api.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/nfc_api.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/pinmap.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/pinmap_common.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/port_api.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/pwmout_api.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/rtc_api.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/serial_api.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/sleep.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/spi_api.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/sys_api.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/timer_api.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/us_ticker.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/us_ticker_api.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/wait_api.c>",
        "+<component/common/mbed/targets/hal/rtl8711b/wdt_api.c>",
        "+<component/soc/realtek/8711b/fwlib/ram_lib/rtl8710b_dsleepcfg.c>",
        "+<component/soc/realtek/8711b/fwlib/ram_lib/rtl8710b_dstandbycfg.c>",
        "+<component/soc/realtek/8711b/fwlib/ram_lib/rtl8710b_intfcfg.c>",
        "+<component/soc/realtek/8711b/misc/rtl8710b_ota.c>",
        "+<component/soc/realtek/8711b/fwlib/ram_lib/rtl8710b_pinmapcfg.c>",
        "+<component/soc/realtek/8711b/fwlib/ram_lib/rtl8710b_sleepcfg.c>",
        "+<component/common/utilities/cJSON.c>",
        "+<component/common/utilities/xml.c>",
    ],
    includes=[
        "+<project/realtek_amebaz_va0_example/inc>",
        "+<component/os/freertos>",
        "+<component/os/os_dep/include>",
        "+<component/common/api/network/include>",
        "+<component/common/api>",
        "+<component/common/api/platform>",
        "+<component/common/api/wifi>",
        "+<component/common/api/wifi/rtw_wpa_supplicant/src>",
        "+<component/common/api/wifi/rtw_wowlan>",
        "+<component/common/api/wifi/rtw_wpa_supplicant/wpa_supplicant>",
        "+<component/common/application>",
        "+<component/common/drivers/wlan/realtek/include>",
        "+<component/common/drivers/wlan/realtek/src/osdep>",
        "+<component/common/drivers/wlan/realtek/wlan_ram_map/rom>",
        "+<component/common/file_system>",
        "+<component/common/network>",
        "+<component/common/network/ssl/mbedtls-2.4.0/include>",
        "+<component/common/network/ssl/ssl_ram_map/rom>",
        "+<component/common/utilities>",
        "+<component/soc/realtek/8711b/app/monitor/include>",
        "+<component/soc/realtek/8711b/cmsis>",
        "+<component/soc/realtek/8711b/cmsis/device>",
        "+<component/soc/realtek/8711b/fwlib>",
        "+<component/soc/realtek/8711b/fwlib/include>",
        "+<component/soc/realtek/8711b/swlib/std_lib/include>",
        "+<component/soc/realtek/8711b/swlib/std_lib/libc/rom/string>",
        "+<component/soc/realtek/8711b/swlib/rtl_lib>",
        "+<component/soc/realtek/8711b/misc>",
        "+<component/common/mbed/api>",
        "+<component/common/mbed/hal>",
        "+<component/common/mbed/hal_ext>",
        "+<component/common/mbed/targets/cmsis>",
        "+<component/common/mbed/targets/hal/rtl8711b>",
        # keep PolarSSL headers for ROM crypto functions
        "+<component/common/network/ssl/polarssl-1.3.8/include>",
        # includes that are missing in the vanilla SDK makefiles
        "+<component/common/drivers/sdio/realtek/sdio_host/inc>",
        "+<component/common/file_system/fatfs>",
        "+<component/common/file_system/fatfs/r0.10c/include>",
        "+<component/common/network/mdns>",
        "+<component/common/network/libwsclient>",
    ],
    options=dict(
        CFLAGS=["-w"],
    ),
)

# Sources - FreeRTOS
env.Replace(FREERTOS_PORT=env["FAMILY_NAME"], FREERTOS_PORT_DEFINE="REALTEK_AMB1")
queue.AddExternalLibrary("freertos")
queue.AddExternalLibrary("freertos-port")

# Sources - lwIP
queue.AddExternalLibrary("lwip", port="amb1")

# Sources - mbedTLS
queue.AddLibrary(
    name="ambz_mbedtls",
    base_dir="$SDK_DIR",
    srcs=[
        # mbedTLS from SDK
        "+<component/common/network/ssl/mbedtls-2.4.0/library/*.c>",
        # replace this with a fixup
        "-<component/common/network/ssl/mbedtls-2.4.0/library/net_sockets.c>",
    ],
    includes=[
        "+<component/common/network/ssl/mbedtls-2.4.0/include>",
    ],
)

# Libs & linker config
queue.AppendPublic(
    LIBPATH=[
        # fmt: off
        join("$SDK_DIR", "component", "soc", "realtek", "8711b", "misc", "bsp", "lib", "common", "GCC"),
        # fmt: on
    ],
    LIBS=[
        "_platform",
        "_wlan",
        "_wps",
        "_p2p",
        "_dct",
        # use lib_rtlstd.a without some __aeabi functions
        "_rtlstd_patch",
        "m",
        "c",
        "nosys",
        "gcc",
        "_websocket",
        "_http",
        "_mdns",
    ],
)

# Misc options
env.Replace(
    SIZEPROGREGEXP=r"^(?:\.ram_image2\.entry|\.ram_image2\.text|\.ram_image2\.data|\.xip_image2\.text)\s+([0-9]+).*",
    SIZEDATAREGEXP=r"^(?:\.ram_image2\.entry|\.ram_image2\.data|\.ram_image2\.bss|\.ram_image2\.skb\.bss)\s+([0-9]+).*",
    SIZECHECKCMD="$SIZETOOL -A -d $SOURCES",
    SIZEPRINTCMD="$SIZETOOL -B -d $SOURCES",
)
# Generate linker scripts with correct flash offsets
env.GenerateLinkerScript(board, board.get("build.ldscript"))
env.GenerateLinkerScript(board, board.get("build.ldscript").replace("xip1", "xip2"))

env.Append(
    BUILDERS=dict(
        BinToObj=Builder(
            action=" ".join(
                [
                    "$OBJCOPY",
                    "-I binary",
                    "-O elf32-littlearm",
                    "-B arm",
                    "$SOURCE",
                    "$TARGET",
                ],
            ),
        )
    ),
)

# Bootloader library
boot_all = board.get("build.amb_boot_all")
target_boot = env.StaticLibrary(
    join("$BUILD_DIR", "boot_all"),
    env.BinToObj(
        join("$BUILD_DIR", "boot_all.o"),
        join("$MISC_DIR", boot_all),
    ),
)
env.Prepend(LIBS=[target_boot])

# Build all libraries
queue.BuildLibraries()

# Main firmware outputs and actions
image_ota1 = "${BUILD_DIR}/image_ota1.${FLASH_OTA1_OFFSET}.bin"
image_ota2 = "${BUILD_DIR}/image_ota2.${FLASH_OTA2_OFFSET}.bin"
env.Replace(
    # linker command (dual .bin outputs)
    LINK="${LTCHIPTOOL} link2bin ${BOARD_JSON} xip1 xip2",
    # UF2OTA input list
    UF2OTA=[
        # same OTA images for flasher and device
        f"{image_ota1},{image_ota2}=device:ota1,ota2;flasher:ota1,ota2",
    ],
)

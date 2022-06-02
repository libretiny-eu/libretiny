# Copyright (c) Kuba Szczodrzyński 2022-04-20.

import sys
from os.path import join

from SCons.Script import Builder, DefaultEnvironment

env = DefaultEnvironment()
board = env.BoardConfig()

env.AddDefaults("realtek-ambz", "framework-realtek-amb1")

# Flags
env.Append(
    CFLAGS=[
        "-std=gnu99",
        "-mcpu=cortex-m4",
        "-mthumb",
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16",
        "-g2",
        "-w",
        "-O2",
        "-Wno-pointer-sign",
        "-fdata-sections",
        "-ffunction-sections",
        "-fmessage-length=0",
        "-fno-common",
        "-fno-short-enums",
        "-fomit-frame-pointer",
        "-fsigned-char",
    ],
    CPPDEFINES=[
        "M3",
        "CONFIG_PLATFORM_8711B",
        # LwIP options
        ("LWIP_TIMEVAL_PRIVATE", "0"),
        ("LWIP_NETIF_HOSTNAME", "1"),  # to support hostname changing
        ("LWIP_SO_RCVBUF", "1"),  # for ioctl(FIONREAD)
        ("ip_addr", "ip4_addr"),  # LwIP 2.0.x compatibility
        ("ip_addr_t", "ip4_addr_t"),  # LwIP 2.0.x compatibility
        ("IN_ADDR_T_DEFINED", "1"),
        ("in_addr_t", "u32_t"),
        ("INT_MAX", "2147483647"),  # for RECV_BUFSIZE_DEFAULT
        ("ERRNO", "1"),  # for LwIP
        ("vprintf", "rtl_vprintf"),
        "MBEDTLS_KEY_EXCHANGE__SOME__PSK_ENABLED",  # enable PSK in mbedTLS
        # "MBEDTLS_DEBUG_C",
    ],
    LINKFLAGS=[
        "-mcpu=cortex-m4",
        "-mthumb",
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16",
        "-g",
        "--specs=nano.specs",
        "-nostartfiles",
        "-Wl,-Map=" + join("$BUILD_DIR", "${PROGNAME}.map"),
        "-Os",
        "-Wl,--gc-sections",
        "-Wl,--cref",
        "-Wl,--entry=Reset_Handler",
        "-Wl,--no-enum-size-warning",
        "-Wl,--no-wchar-size-warning",
        "-Wl,-wrap,malloc",
        "-Wl,-wrap,free",
        "-Wl,-wrap,realloc",
        "-Wl,-wrap,rom_psk_CalcGTK",
        "-Wl,-wrap,rom_psk_CalcPTK",
        "-Wl,-wrap,CalcMIC",
        "-Wl,-wrap,CheckMIC",
        "-Wl,-wrap,aes_80211_encrypt",
        "-Wl,-wrap,aes_80211_decrypt",
        "-Wl,-wrap,DecGTK",
    ],
)

# Sources - from SDK project/realtek_amebaz_va0_example/GCC-RELEASE/application.mk
# - "console" is disabled as it introduces build error, and is generally useless
# - "utilities example" are also not really needed
env.AddLibrary(
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
        "+<component/common/api/wifi/wifi_ind.c>",
        "+<component/common/api/wifi/wifi_promisc.c>",
        "+<component/common/api/wifi/wifi_simple_config.c>",
        "+<component/common/api/wifi/wifi_util.c>",
        "+<component/common/api/lwip_netconf.c>",
        "+<component/common/drivers/wlan/realtek/src/osdep/lwip_intf.c>",
        "+<component/common/network/dhcp/dhcps.c>",
        "+<component/common/network/sntp/sntp.c>",
        "+<component/common/network/ssl/ssl_ram_map/ssl_ram_map.c>",
        "+<component/os/freertos/freertos_v8.1.2/Source/portable/MemMang/heap_5.c>",
        "+<component/os/freertos/freertos_v8.1.2/Source/portable/GCC/ARM_CM4F/port.c>",
        # "+<component/os/freertos/freertos_v8.1.2/Source/portable/IAR/ARM_CM4F/portasm.s>",
        "+<component/os/freertos/cmsis_os.c>",
        "+<component/os/freertos/freertos_v8.1.2/Source/croutine.c>",
        "+<component/os/freertos/freertos_v8.1.2/Source/event_groups.c>",
        "+<component/os/freertos/freertos_service.c>",
        "+<component/os/freertos/freertos_v8.1.2/Source/list.c>",
        "+<component/os/freertos/freertos_v8.1.2/Source/queue.c>",
        "+<component/os/freertos/freertos_v8.1.2/Source/tasks.c>",
        "+<component/os/freertos/freertos_v8.1.2/Source/timers.c>",
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
        "+<component/os/freertos/freertos_v8.1.2/Source/include>",
        "+<component/os/freertos/freertos_v8.1.2/Source/portable/GCC/ARM_CM4F>",
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
    ],
)

# Sources - lwIP 2.0.0
env.AddLibraryLwIP(version="2.0.0", port="amb1")

# Sources - mbedTLS
env.AddLibrary(
    name="ambz_mbedtls",
    base_dir="$SDK_DIR",
    srcs=[
        # mbedTLS from SDK
        "+<component/common/network/ssl/mbedtls-2.4.0/library/*.c>",
        # replace these with fixups
        "-<component/common/network/ssl/mbedtls-2.4.0/library/net_sockets.c>",
        "-<component/common/network/ssl/mbedtls-2.4.0/library/ssl_tls.c>",
    ],
    includes=[
        "+<component/common/network/ssl/mbedtls-2.4.0/include>",
    ],
)

# Sources - family fixups
env.AddLibrary(
    name="ambz_fixups",
    base_dir="$FIXUPS_DIR",
    srcs=[
        "+<app_start_patch.c>",
        "+<cmsis_ipsr.c>",
        "+<log_uart.c>",
        "+<net_sockets.c>",  # fix non-blocking sockets (Realtek disabled this for unknown reason)
        "+<ssl_tls.c>",  # rtl sdk defines S1 and S2 which conflicts here
        "+<wifi_mode.c>",
    ],
)

# Libs & linker config
env.Append(
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
        "_rtlstd",
        "m",
        "c",
        "nosys",
        "gcc",
        "_websocket",
        "_http",
        "_mdns",
    ],
)
env.Replace(
    LDSCRIPT_PATH=[
        join("$LD_DIR", "$LDSCRIPT_SDK"),
    ],
)

# Misc options
env.Replace(
    SIZEPROGREGEXP=r"^(?:\.ram_image2\.entry|\.ram_image2\.text|\.ram_image2\.data|\.xip_image2\.text)\s+([0-9]+).*",
    SIZEDATAREGEXP=r"^(?:\.ram_image2\.entry|\.ram_image2\.data|\.ram_image2\.bss|\.ram_image2\.skb\.bss)\s+([0-9]+).*",
    SIZECHECKCMD="$SIZETOOL -A -d $SOURCES",
    SIZEPRINTCMD="$SIZETOOL -B -d $SOURCES",
)

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

# Uploader
upload_protocol = env.subst("$UPLOAD_PROTOCOL")
upload_source = ""
upload_actions = []
# from platform-espressif32/builder/main.py
if upload_protocol == "uart":
    env.Replace(
        UPLOADER=join("$TOOLS_DIR", "rtltool.py"),
        UPLOADERFLAGS=[
            "--port",
            "$UPLOAD_PORT",
            "--go",  # run firmware after uploading
            "wf",  # Write a binary file to Flash data
        ],
        UPLOADCMD='"$PYTHONEXE" "$UPLOADER" $UPLOADERFLAGS $FLASH_OTA1_OFFSET "$BUILD_DIR/$IMG_FW"',
    )
    upload_actions = [
        env.VerboseAction(env.AutodetectUploadPort, "Looking for upload port..."),
        env.VerboseAction("$UPLOADCMD", "Uploading $IMG_FW"),
    ]
elif upload_protocol == "custom":
    upload_actions = [env.VerboseAction("$UPLOADCMD", "Uploading $IMG_FW")]
else:
    sys.stderr.write("Warning! Unknown upload protocol %s\n" % upload_protocol)

# Bootloader library
boot_all = board.get("build.amb_boot_all")
target_boot = env.StaticLibrary(
    join("$BUILD_DIR", "boot_all"),
    env.BinToObj(
        join("$BUILD_DIR", "boot_all.o"),
        join("$BIN_DIR", boot_all),
    ),
)
env.Prepend(LIBS=[target_boot])

# Build all libraries
env.BuildLibraries()

# Main firmware outputs and actions
env.Replace(
    # linker command (dual .bin outputs)
    LINK="${LINK2BIN} AMBZ xip1 xip2",
    # default output .bin name
    IMG_FW="image_${FLASH_OTA1_OFFSET}.ota1.bin",
    # UF2OTA input list
    UF2OTA=[
        (
            "ota1",
            "${BUILD_DIR}/image_${FLASH_OTA1_OFFSET}.ota1.bin",
            "ota2",
            "${BUILD_DIR}/image_${FLASH_OTA2_OFFSET}.ota2.bin",
        ),
    ],
    # uploader
    UPLOAD_ACTIONS=upload_actions,
)

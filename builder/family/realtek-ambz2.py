# Copyright (c) Kuba Szczodrzyński 2022-07-20.

from os.path import join

from platformio.platform.board import PlatformBoardConfig
from SCons.Script import DefaultEnvironment, Environment

env: Environment = DefaultEnvironment()
board: PlatformBoardConfig = env.BoardConfig()

COMPONENT_DIR = join("$SDK_DIR", "component")

# Flags
env.Append(
    CCFLAGS=[
        "-march=armv8-m.main+dsp",
        "-mthumb",
        "-mcmse",
        "-mfloat-abi=soft",
        "-g",
        "-gdwarf-3",
        "-Os",
        "-MMD",
        "-fstack-usage",
        "-fdata-sections",
        "-ffunction-sections",
        "-fmessage-length=0",
        "-fno-common",
        "-fno-short-enums",
        "-fomit-frame-pointer",
        "-fsigned-char",
    ],
    CFLAGS=[
        "-std=gnu99",
        "-Wall",
        "-Wpointer-arith",
        "-Wundef",
        "-Wno-write-strings",
        "-Wno-maybe-uninitialized",
    ],
    CXXFLAGS=[
        "-std=c++11",
        "-fno-exceptions",
        "-fno-rtti",
        "-fno-use-cxa-atexit",
    ],
    CPPDEFINES=[
        # other options
        "__thumb2__",
        "CONFIG_PLATFORM_8710C",
        ("__ARM_ARCH_8M_MAIN__", "1"),
        ("CONFIG_BUILD_RAM", "1"),
        "V8M_STKOVF",
    ],
    LINKFLAGS=[
        "-march=armv8-m.main+dsp",
        "-mthumb",
        "-mcmse",
        "-mfloat-abi=soft",
        "-g",
        "--specs=nosys.specs",
        "-nostartfiles",
        "-nodefaultlibs",
        "-nostdlib",
        "-Os",
        "-Wl,--gc-sections",
        "-Wl,--warn-section-align",
        "-Wl,--cref",
        "-Wl,--build-id=none",
        "-Wl,--use-blx",
        "-Wl,-no-enum-size-warning",
        # TODO fix wraps
        "-Wl,-wrap,strcat",
        "-Wl,-wrap,strchr",
        "-Wl,-wrap,strcmp",
        "-Wl,-wrap,strncmp",
        "-Wl,-wrap,strnicmp",
        "-Wl,-wrap,strcpy",
        "-Wl,-wrap,strncpy",
        "-Wl,-wrap,strlcpy",
        "-Wl,-wrap,strlen",
        "-Wl,-wrap,strnlen",
        "-Wl,-wrap,strncat",
        "-Wl,-wrap,strpbrk",
        "-Wl,-wrap,strspn",
        "-Wl,-wrap,strstr",
        "-Wl,-wrap,strtok",
        "-Wl,-wrap,strxfrm",
        "-Wl,-wrap,strsep",
        "-Wl,-wrap,strtod",
        "-Wl,-wrap,strtof",
        "-Wl,-wrap,strtold",
        "-Wl,-wrap,strtoll",
        "-Wl,-wrap,strtoul",
        "-Wl,-wrap,strtoull",
        "-Wl,-wrap,atoi",
        "-Wl,-wrap,atoui",
        "-Wl,-wrap,atol",
        "-Wl,-wrap,atoul",
        "-Wl,-wrap,atoull",
        "-Wl,-wrap,atof",
        "-Wl,-wrap,malloc",
        "-Wl,-wrap,realloc",
        "-Wl,-wrap,calloc",
        "-Wl,-wrap,free",
        "-Wl,-wrap,_malloc_r",
        "-Wl,-wrap,_calloc_r",
        "-Wl,-wrap,memcmp",
        "-Wl,-wrap,memcpy",
        "-Wl,-wrap,memmove",
        "-Wl,-wrap,memset",
        "-Wl,-wrap,printf",
        "-Wl,-wrap,sprintf",
        "-Wl,-wrap,puts",
        "-Wl,-wrap,putc",
        "-Wl,-wrap,putchar",
        "-Wl,-wrap,snprintf",
        "-Wl,-wrap,vsnprintf",
        "-Wl,-wrap,aesccmp_construct_mic_iv",
        "-Wl,-wrap,aesccmp_construct_mic_header1",
        "-Wl,-wrap,aesccmp_construct_ctr_preload",
        "-Wl,-wrap,rom_psk_CalcGTK",
        "-Wl,-wrap,rom_psk_CalcPTK",
        "-Wl,-wrap,aes_80211_encrypt",
        "-Wl,-wrap,aes_80211_decrypt",
    ],
)

# Sources - from SDK project/realtek_amebaz2_v0_example/GCC-RELEASE/application.is.mk
# - without "utilities - example", "bluetooth - example" and "network - app - mqtt"
env.AddLibrary(
    name="ambz2_sdk",
    base_dir=COMPONENT_DIR,
    srcs=[
        # libc api wrapper
        "+<soc/realtek/8710c/misc/utilities/source/ram/libc_wrap.c>",
        # cmsis
        "+<soc/realtek/8710c/cmsis/rtl8710c/source/ram/*.c>",
        "+<soc/realtek/8710c/cmsis/rtl8710c/source/ram_s/app_start.c>",
        # console
        "+<common/api/at_cmd/atcmd_bt.c>",
        "+<common/api/at_cmd/atcmd_lwip.c>",
        "+<common/api/at_cmd/atcmd_mp_ext2.c>",
        "+<common/api/at_cmd/atcmd_mp.c>",
        "+<common/api/at_cmd/atcmd_sys.c>",
        "+<common/api/at_cmd/atcmd_wifi.c>",
        "+<common/api/at_cmd/log_service.c>",
        "+<soc/realtek/8710c/app/shell/cmd_shell.c>",
        "+<soc/realtek/8710c/app/shell/ram_s/consol_cmds.c>",
        "+<soc/realtek/8710c/misc/driver/rtl_console.c>",
        # utilities
        "+<common/utilities/cJSON.c>",
        "+<common/utilities/http_client.c>",
        "+<common/utilities/xml.c>",
        # os
        "+<os/freertos/cmsis_os.c>",
        "+<os/freertos/freertos_cb.c>",
        "+<os/freertos/freertos_pmu.c>",
        "+<os/freertos/freertos_service.c>",
        "+<os/os_dep/device_lock.c>",
        "+<os/os_dep/osdep_service.c>",
        # os - freertos
        "+<os/freertos/freertos_v10.0.1/Source/*.c>",
        # os - freertos - portable
        "+<os/freertos/freertos_v10.0.1/Source/portable/MemMang/heap_5.c>",
        "+<os/freertos/freertos_v10.0.1/Source/portable/GCC/ARM_RTL8710C/port.c>",
        # peripheral - api
        "+<common/mbed/targets/hal/rtl8710c/*.c>",
        # peripheral - hal
        "+<soc/realtek/8710c/fwlib/source/ram/*.c>",
        "+<soc/realtek/8710c/fwlib/source/ram_ns/*.c>",
        "+<soc/realtek/8710c/fwlib/source/ram_s/hal_efuse.c>",
        "+<soc/realtek/8710c/fwlib/source/ram_s/hal_pinmux_nsc.c>",
        # peripheral - wlan
        # "+<common/drivers/wlan/realtek/src/core/option/rtw_opt_rf_para_rtl8710c.c>",
        # file_system - fatfs
        "+<common/file_system/fatfs/disk_if/src/flash_fatfs.c>",
        "+<common/file_system/fatfs/fatfs_ext/src/ff_driver.c>",
        "+<common/file_system/fatfs/r0.10c/src/diskio.c>",
        "+<common/file_system/fatfs/r0.10c/src/ff.c>",
        "+<common/file_system/fatfs/r0.10c/src/option/ccsbcs.c>",
        "+<common/file_system/ftl/ftl.c>",
        # TODO remove this
        "+<common/example/example_entry.c>",
        "+<common/example/wlan_fast_connect/example_wlan_fast_connect.c>",
    ],
    includes=[
        "+<$SDK_DIR/project/realtek_amebaz2_v0_example/inc>",
        "+<common/api/at_cmd>",
        "+<common/api/platform>",
        "+<common/api>",
        "+<common/application>",
        "+<common/example>",
        "+<common/file_system/dct>",
        "+<common/file_system/fatfs/r0.10c/include>",
        "+<common/file_system/fatfs>",
        "+<common/file_system/ftl>",
        "+<common/file_system>",
        "+<common/mbed/hal_ext>",
        "+<common/mbed/hal>",
        "+<common/mbed/targets/hal/rtl8710c>",
        "+<common/media/mmfv2>",
        "+<common/media/rtp_codec>",
        "+<common/test>",
        "+<common/utilities>",
        "+<os/freertos>",
        "+<os/freertos/freertos_v10.0.1/Source/include>",
        "+<os/freertos/freertos_v10.0.1/Source/portable/GCC/ARM_RTL8710C>",
        "+<os/os_dep/include>",
        "+<soc/realtek/8710c/app/rtl_printf/include>",
        "+<soc/realtek/8710c/app/shell>",
        "+<soc/realtek/8710c/app/stdio_port>",
        "+<soc/realtek/8710c/cmsis/cmsis-core/include>",
        "+<soc/realtek/8710c/cmsis/rtl8710c/include>",
        "+<soc/realtek/8710c/cmsis/rtl8710c/lib/include>",
        "+<soc/realtek/8710c/fwlib/include>",
        "+<soc/realtek/8710c/fwlib/lib/include>",
        "+<soc/realtek/8710c/mbed-drivers/include>",
        "+<soc/realtek/8710c/misc/driver>",
        "+<soc/realtek/8710c/misc/os>",
        "+<soc/realtek/8710c/misc/platform>",
        "+<soc/realtek/8710c/misc/utilities/include>",
    ],
    options=dict(
        CCFLAGS=[
            "-Wno-int-conversion",
            "-Wno-unused-label",
            "-Wno-unused-but-set-variable",
            "-Wno-undef",
            "-Wno-pointer-sign",
            "-Wno-parentheses",
            "-Wno-implicit-function-declaration",
            "-Wno-misleading-indentation",
            # TODO remove this; only for example_wlan_fast_connect.c
            "-Wno-format-truncation",
            "-Wno-return-type",
        ],
    ),
)

# Sources - network utilities
env.AddLibrary(
    name="ambz2_net",
    base_dir=COMPONENT_DIR,
    srcs=[
        # network - api
        "+<common/api/lwip_netconf.c>",
        # network - api - wifi
        "+<common/api/wifi/*.c>",
        # network - api - wifi - rtw_wpa_supplicant
        "+<common/api/wifi/rtw_wpa_supplicant/src/crypto/tls_polarssl.c>",
        "+<common/api/wifi/rtw_wpa_supplicant/wpa_supplicant/*.c>",
        # network - app
        "+<soc/realtek/8710c/misc/platform/ota_8710c.c>",
        "+<common/api/network/src/ping_test.c>",
        "+<common/utilities/ssl_client.c>",
        "+<common/utilities/ssl_client_ext.c>",
        "+<common/utilities/tcptest.c>",
        "+<common/api/network/src/wlan_network.c>",
        # network - coap
        "+<common/network/coap/*.c>",
        # network - http
        "+<common/network/httpc/httpc_tls.c>",
        "+<common/network/httpd/httpd_tls.c>",
        # network
        "+<common/network/dhcp/dhcps.c>",
        "+<common/network/sntp/sntp.c>",
        # network - websocket
        "+<common/network/websocket/*.c>",
        # network - mdns
        "+<common/network/mDNS/mDNSPlatform.c>",
        # network - lwip - port
        "+<common/drivers/wlan/realtek/src/osdep/lwip_intf.c>",
        # network - ssl - ssl_ram_map
        "+<common/network/ssl/ssl_ram_map/rom/rom_ssl_ram_map.c>",
        "+<common/network/ssl/ssl_func_stubs/ssl_func_stubs.c>",
    ],
    includes=[
        "+<common/api/network/include>",
        "+<common/api/wifi>",
        "+<common/api/wifi/rtw_wpa_supplicant/src/crypto>",
        "+<common/api/wifi/rtw_wpa_supplicant/src>",
        "+<common/drivers/wlan/realtek/include>",
        "+<common/drivers/wlan/realtek/src/core/option>",
        "+<common/drivers/wlan/realtek/src/osdep>",
        "+<common/network>",
        "+<common/network/coap/include>",
        "+<common/network/http2/nghttp2-1.31.0/includes>",
        "+<common/network/libcoap/include>",
        "+<common/network/ssl/ssl_ram_map/rom>",
    ],
    options=dict(
        CCFLAGS=[
            "-Wno-pointer-sign",
            "-Wno-unused-value",
            "-Wno-format",
            "-Wno-implicit-function-declaration",
            "-Wno-unused-function",
            "-Wno-parentheses",
            "-Wno-incompatible-pointer-types",
            "-Wno-array-bounds",
            "-Wno-stringop-overflow",
        ],
    ),
)

# Sources - Bluetooth support
env.AddLibrary(
    name="ambz2_bluetooth",
    base_dir=join(COMPONENT_DIR, "common", "bluetooth", "realtek", "sdk"),
    srcs=[
        "+<board/**/*.c>",
        "+<src/ble/profile/client/*.c>",
        "+<src/ble/profile/server/bas.c>",
        "+<src/ble/profile/server/dis.c>",
        "+<src/ble/profile/server/hids.c>",
        # "+<src/ble/profile/server/hids_kb.c>",
        # "+<src/ble/profile/server/hids_rmc.c>",
        "+<src/ble/profile/server/simple_ble_service.c>",
        "+<src/mcu/module/data_uart_cmd/user_cmd_parse.c>",
    ],
    includes=[
        "+<.>",
        "+<board/amebaz2/lib>",
        "+<board/amebaz2/src/data_uart>",
        "+<board/amebaz2/src/hci>",
        "+<board/amebaz2/src/os>",
        "+<board/amebaz2/src/vendor_cmd>",
        "+<board/amebaz2/src>",
        "+<board/common/inc>",
        "+<inc>",
        "+<inc/app>",
        "+<inc/bluetooth/gap>",
        "+<inc/bluetooth/profile/client>",
        "+<inc/bluetooth/profile/server>",
        "+<inc/bluetooth/profile>",
        "+<inc/os>",
        "+<inc/platform>",
        "+<inc/stack>",
        "+<src/mcu/module/data_uart_cmd>",
    ],
    options=dict(
        CCFLAGS=[
            "-Wno-unused-function",
            "-Wno-unused-variable",
            "-Wno-implicit-function-declaration",
        ],
    ),
)


# Sources - lwIP 2.0.2
env.AddLibrary(
    name="ambz2_lwip",
    base_dir=join(COMPONENT_DIR, "common", "network", "lwip", "lwip_v2.0.2"),
    srcs=[
        "+<port/realtek/freertos/*.c>",
        "+<src/api/*.c>",
        "+<src/apps/ping/*.c>",
        "+<src/apps/mdns/*.c>",
        "+<src/core/*.c>",
        "+<src/core/ipv4/*.c>",
        "+<src/core/ipv6/*.c>",
        "+<src/netif/ethernet.c>",
    ],
    includes=[
        "+<port/realtek>",
        "+<port/realtek/freertos>",
        "+<src/include>",
        "+<src/include/netif>",
    ],
)

# Sources - mbedTLS
env.AddLibrary(
    name="ambz2_mbedtls",
    base_dir=join(COMPONENT_DIR, "common", "network", "ssl", "mbedtls-2.4.0"),
    srcs=[
        "+<library/aesni.c>",
        "+<library/blowfish.c>",
        "+<library/camellia.c>",
        "+<library/ccm.c>",
        "+<library/certs.c>",
        "+<library/cipher_wrap.c>",
        "+<library/cipher.c>",
        "+<library/cmac.c>",
        "+<library/debug.c>",
        "+<library/error.c>",
        "+<library/gcm.c>",
        "+<library/havege.c>",
        "+<library/md_wrap.c>",
        "+<library/md.c>",
        "+<library/md2.c>",
        "+<library/md4.c>",
        "+<library/memory_buffer_alloc.c>",
        "+<library/net_sockets.c>",
        "+<library/padlock.c>",
        "+<library/pkcs11.c>",
        "+<library/pkcs12.c>",
        "+<library/pkcs5.c>",
        "+<library/pkparse.c>",
        "+<library/platform.c>",
        "+<library/ripemd160.c>",
        "+<library/sha256.c>",
        "+<library/ssl_cache.c>",
        "+<library/ssl_ciphersuites.c>",
        "+<library/ssl_cli.c>",
        "+<library/ssl_cookie.c>",
        "+<library/ssl_srv.c>",
        "+<library/ssl_ticket.c>",
        "+<library/ssl_tls.c>",
        "+<library/threading.c>",
        "+<library/timing.c>",
        "+<library/version_features.c>",
        "+<library/version.c>",
        "+<library/x509_create.c>",
        "+<library/x509_crl.c>",
        "+<library/x509_crt.c>",
        "+<library/x509_csr.c>",
        "+<library/x509.c>",
        "+<library/x509write_crt.c>",
        "+<library/x509write_csr.c>",
        "+<library/xtea.c>",
    ],
    includes=[
        "+<include>",
    ],
)

# Libs & linker config
env.Append(
    LIBPATH=[
        # fmt: off
        join(COMPONENT_DIR, "soc", "realtek", "8710c", "misc", "bsp", "lib", "common", "GCC"),
        join(COMPONENT_DIR, "soc", "realtek", "8710c", "fwlib", "lib", "lib"),
        join(COMPONENT_DIR, "common", "bluetooth", "realtek", "sdk", "board", "amebaz2", "lib"),
        join(COMPONENT_DIR, "soc", "realtek", "8710c", "misc", "bsp", "ROM"),
        # fmt: on
    ],
    LIBS=[
        "_soc_is",
        "_wlan",
        "_http",
        "_dct",
        "_eap",
        "_p2p",
        "_websocket",
        "_wps",
        "m",
        "c",
        "nosys",
        "gcc",
        # SCons trims the .a suffix automatically
        ":hal_pmc.a.a",
        ":btgap.a.a",
    ],
)

# Misc options
env.Replace(
    SIZEPROGREGEXP=r"^(?:\.ram\..*?|\.psram\.[cd].*?|\.data|\.xip[\._].*?)\s+([0-9]+).*",
    SIZEDATAREGEXP=r"^(?:\.ram\..*?|\.data)\s+([0-9]+).*",
    SIZECHECKCMD="$SIZETOOL -A -d $SOURCES",
    SIZEPRINTCMD="$SIZETOOL -B -d $SOURCES",
)

# Build all libraries
env.BuildLibraries()

# Main firmware outputs and actions
env.Replace(
    # TODO
)

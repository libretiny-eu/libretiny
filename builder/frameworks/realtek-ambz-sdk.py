from os.path import isdir, join

from SCons.Script import Builder, DefaultEnvironment

env = DefaultEnvironment()
platform = env.PioPlatform()
board = env.BoardConfig()

mcu = board.get("build.mcu").upper()
family = board.get("build.family").upper()
variant = board.get("build.variant")
ldscript = board.get("build.ldscript_sdk")

SDK_DIR = platform.get_package_dir("framework-realtek-amb1")
BOARD_DIR = join(platform.get_dir(), "boards", variant)
FIXUPS_DIR = join(platform.get_dir(), "fixups", "realtek-ambz")
PLATFORM_DIR = join(platform.get_dir(), "platform", "realtek-ambz")
assert isdir(SDK_DIR)
assert isdir(env.subst(BOARD_DIR))
assert isdir(env.subst(FIXUPS_DIR))
assert isdir(env.subst(PLATFORM_DIR))

ota1_offset = board.get("build.amb_ota1_offset")
ota2_offset = board.get("build.amb_ota2_offset")
boot_all = board.get("build.amb_boot_all")

# Outputs
env.Replace(
    IMG_FW="image2_all_ota1.bin",
    IMG_OTA="ota_all.bin",
)

# Tools
# fmt: off
TOOL_DIR = join(SDK_DIR, "component", "soc", "realtek", "8711b", "misc", "iar_utility", "common", "tools")
# fmt: on
assert isdir(SDK_DIR)
env.Replace(
    PICK=join(TOOL_DIR, "pick"),
    PAD=join(TOOL_DIR, "pad"),
    CHECKSUM=join(TOOL_DIR, "checksum"),
    OTA=join(TOOL_DIR, "ota"),
)

# Flags
env.Replace(
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
        ("F_CPU", "166000000L"),
        ("LWIP_TIMEVAL_PRIVATE", "0"),
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

# Includes
env.Append(
    CPPPATH=[
        # fmt: off
        join(BOARD_DIR),
        join(FIXUPS_DIR),
        join(PLATFORM_DIR),
        join(SDK_DIR, "project", "realtek_amebaz_va0_example", "inc"),
        join(SDK_DIR, "component", "os", "freertos"),
        join(SDK_DIR, "component", "os", "freertos", "freertos_v8.1.2", "Source", "include"),
        join(SDK_DIR, "component", "os", "freertos", "freertos_v8.1.2", "Source", "portable", "GCC", "ARM_CM4F"),
        join(SDK_DIR, "component", "os", "os_dep", "include"),
        join(SDK_DIR, "component", "common", "api", "network", "include"),
        join(SDK_DIR, "component", "common", "api"),
        join(SDK_DIR, "component", "common", "api", "at_cmd"),
        join(SDK_DIR, "component", "common", "api", "platform"),
        join(SDK_DIR, "component", "common", "api", "wifi"),
        join(SDK_DIR, "component", "common", "api", "wifi", "rtw_wpa_supplicant", "src"),
        join(SDK_DIR, "component", "common", "api", "wifi", "rtw_wowlan"),
        join(SDK_DIR, "component", "common", "api", "wifi", "rtw_wpa_supplicant", "wpa_supplicant"),
        join(SDK_DIR, "component", "common", "application"),
        join(SDK_DIR, "component", "common", "application", "mqtt", "MQTTClient"),
        join(SDK_DIR, "component", "common", "application", "mqtt", "MQTTPacket"),
        join(SDK_DIR, "component", "common", "example"),
        join(SDK_DIR, "component", "common", "example", "wlan_fast_connect"),
        join(SDK_DIR, "component", "common", "drivers", "modules"),
        join(SDK_DIR, "component", "common", "drivers", "sdio", "realtek", "sdio_host", "inc"),
        join(SDK_DIR, "component", "common", "drivers", "inic", "rtl8711b"),
        join(SDK_DIR, "component", "common", "drivers", "usb_class", "device"),
        join(SDK_DIR, "component", "common", "drivers", "usb_class", "device", "class"),
        join(SDK_DIR, "component", "common", "drivers", "wlan", "realtek", "include"),
        join(SDK_DIR, "component", "common", "drivers", "wlan", "realtek", "src", "osdep"),
        join(SDK_DIR, "component", "common", "drivers", "wlan", "realtek", "src", "hci"),
        join(SDK_DIR, "component", "common", "drivers", "wlan", "realtek", "src", "hal"),
        join(SDK_DIR, "component", "common", "drivers", "wlan", "realtek", "src", "hal", "rtl8711b"),
        join(SDK_DIR, "component", "common", "drivers", "wlan", "realtek", "src", "hal", "OUTSRC"),
        join(SDK_DIR, "component", "common", "drivers", "wlan", "realtek", "wlan_ram_map", "rom"),
        join(SDK_DIR, "component", "common", "file_system"),
        join(SDK_DIR, "component", "common", "network"),
        join(SDK_DIR, "component", "common", "network", "lwip", "lwip_v1.4.1", "port", "realtek", "freertos"),
        join(SDK_DIR, "component", "common", "network", "lwip", "lwip_v1.4.1", "src", "include"),
        join(SDK_DIR, "component", "common", "network", "lwip", "lwip_v1.4.1", "src", "include", "lwip"),
        join(SDK_DIR, "component", "common", "network", "lwip", "lwip_v1.4.1", "src", "include", "ipv4"),
        join(SDK_DIR, "component", "common", "network", "lwip", "lwip_v1.4.1", "port", "realtek"),
        join(SDK_DIR, "component", "common", "network", "ssl", "polarssl-1.3.8", "include"),
        join(SDK_DIR, "component", "common", "network", "ssl", "ssl_ram_map", "rom"),
        join(SDK_DIR, "component", "common", "test"),
        join(SDK_DIR, "component", "common", "utilities"),
        join(SDK_DIR, "component", "soc", "realtek", "8711b", "app", "monitor", "include"),
        join(SDK_DIR, "component", "soc", "realtek", "8711b", "cmsis"),
        join(SDK_DIR, "component", "soc", "realtek", "8711b", "cmsis", "device"),
        join(SDK_DIR, "component", "soc", "realtek", "8711b", "fwlib"),
        join(SDK_DIR, "component", "soc", "realtek", "8711b", "fwlib", "include"),
        join(SDK_DIR, "component", "soc", "realtek", "8711b", "fwlib", "ram_lib", "crypto"),
        join(SDK_DIR, "component", "soc", "realtek", "8711b", "fwlib", "rom_lib"),
        join(SDK_DIR, "component", "soc", "realtek", "8711b", "swlib", "os_dep", "include"),
        join(SDK_DIR, "component", "soc", "realtek", "8711b", "swlib", "std_lib", "include"),
        join(SDK_DIR, "component", "soc", "realtek", "8711b", "swlib", "std_lib", "libc", "include"),
        join(SDK_DIR, "component", "soc", "realtek", "8711b", "swlib", "std_lib", "libc", "rom", "string"),
        join(SDK_DIR, "component", "soc", "realtek", "8711b", "swlib", "std_lib", "libgcc", "rtl8195a", "include"),
        join(SDK_DIR, "component", "soc", "realtek", "8711b", "swlib", "rtl_lib"),
        join(SDK_DIR, "component", "soc", "realtek", "8711b", "misc"),
        join(SDK_DIR, "component", "soc", "realtek", "8711b", "misc", "os"),
        join(SDK_DIR, "component", "common", "mbed", "api"),
        join(SDK_DIR, "component", "common", "mbed", "hal"),
        join(SDK_DIR, "component", "common", "mbed", "hal_ext"),
        join(SDK_DIR, "component", "common", "mbed", "targets", "cmsis"),
        join(SDK_DIR, "component", "common", "mbed", "targets", "hal", "rtl8711b"),
        join(SDK_DIR, "project", "realtek_8195a_gen_project", "rtl8195a", "sw", "lib", "sw_lib", "mbed", "api"),
        join(SDK_DIR, "component", "common", "application", "mqtt", "MQTTClient"),
        join(SDK_DIR, "component", "common", "network", "websocket"),
        # fmt: on
    ],
)

# Sources - from SDK project/realtek_amebaz_va0_example/GCC-RELEASE/application.mk
# - "console" is disabled as it introduces build error, and is generally useless
# - "utilities example" are also not really needed
sources = [
    # fmt: off
    # app uart_adapter
    "+<" + SDK_DIR +"/component/common/application/uart_adapter/uart_adapter.c>",
    # cmsis
    # NOTE: a fixup is used instead, to remove default main()
    # "+<" + SDK_DIR +"/component/soc/realtek/8711b/cmsis/device/app_start.c>",
    "+<" + SDK_DIR +"/component/soc/realtek/8711b/fwlib/ram_lib/startup.c>",
    "+<" + SDK_DIR +"/component/soc/realtek/8711b/cmsis/device/system_8195a.c>",
    # console
    "+<" + SDK_DIR +"/component/soc/realtek/8711b/app/monitor/ram/rtl_trace.c>",
    # network api wifi rtw_wpa_supplicant
    "+<" + SDK_DIR +"/component/common/api/wifi/rtw_wpa_supplicant/wpa_supplicant/wifi_eap_config.c>",
    "+<" + SDK_DIR +"/component/common/api/wifi/rtw_wpa_supplicant/wpa_supplicant/wifi_p2p_config.c>",
    "+<" + SDK_DIR +"/component/common/api/wifi/rtw_wpa_supplicant/wpa_supplicant/wifi_wps_config.c>",
    # network api wifi
    "+<" + SDK_DIR +"/component/common/api/wifi/wifi_conf.c>",
    "+<" + SDK_DIR +"/component/common/api/wifi/wifi_ind.c>",
    "+<" + SDK_DIR +"/component/common/api/wifi/wifi_promisc.c>",
    "+<" + SDK_DIR +"/component/common/api/wifi/wifi_simple_config.c>",
    "+<" + SDK_DIR +"/component/common/api/wifi/wifi_util.c>",
    # network api
    "+<" + SDK_DIR +"/component/common/api/lwip_netconf.c>",
    # network app
    "+<" + SDK_DIR +"/component/common/application/mqtt/MQTTClient/MQTTClient.c>",
    "+<" + SDK_DIR +"/component/common/application/mqtt/MQTTPacket/MQTTConnectClient.c>",
    "+<" + SDK_DIR +"/component/common/application/mqtt/MQTTPacket/MQTTConnectServer.c>",
    "+<" + SDK_DIR +"/component/common/application/mqtt/MQTTPacket/MQTTDeserializePublish.c>",
    "+<" + SDK_DIR +"/component/common/application/mqtt/MQTTPacket/MQTTFormat.c>",
    "+<" + SDK_DIR +"/component/common/application/mqtt/MQTTClient/MQTTFreertos.c>",
    "+<" + SDK_DIR +"/component/common/application/mqtt/MQTTPacket/MQTTPacket.c>",
    "+<" + SDK_DIR +"/component/common/application/mqtt/MQTTPacket/MQTTSerializePublish.c>",
    "+<" + SDK_DIR +"/component/common/application/mqtt/MQTTPacket/MQTTSubscribeClient.c>",
    "+<" + SDK_DIR +"/component/common/application/mqtt/MQTTPacket/MQTTSubscribeServer.c>",
    "+<" + SDK_DIR +"/component/common/application/mqtt/MQTTPacket/MQTTUnsubscribeClient.c>",
    "+<" + SDK_DIR +"/component/common/application/mqtt/MQTTPacket/MQTTUnsubscribeServer.c>",
    "+<" + SDK_DIR +"/component/common/api/network/src/ping_test.c>",
    "+<" + SDK_DIR +"/component/common/utilities/ssl_client.c>",
    "+<" + SDK_DIR +"/component/common/utilities/tcptest.c>",
    "+<" + SDK_DIR +"/component/common/api/network/src/wlan_network.c>",
    # network lwip api
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/api/api_lib.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/api/api_msg.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/api/err.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/api/netbuf.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/api/netdb.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/api/netifapi.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/api/sockets.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/api/tcpip.c>",
    # network lwip core ipv4
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/ipv4/autoip.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/ipv4/icmp.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/ipv4/igmp.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/ipv4/inet.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/ipv4/inet_chksum.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/ipv4/ip.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/ipv4/ip_addr.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/ipv4/ip_frag.c>",
    # network lwip core
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/def.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/dhcp.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/dns.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/init.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/lwip_timers.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/mem.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/memp.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/netif.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/pbuf.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/raw.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/stats.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/sys.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/tcp.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/tcp_in.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/tcp_out.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/core/udp.c>",
    # network lwip netif
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/src/netif/etharp.c>",
    # network lwip port
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/port/realtek/freertos/ethernetif.c>",
    "+<" + SDK_DIR +"/component/common/drivers/wlan/realtek/src/osdep/lwip_intf.c>",
    "+<" + SDK_DIR +"/component/common/network/lwip/lwip_v1.4.1/port/realtek/freertos/sys_arch.c>",
    # network - wsclient
    "+<" + SDK_DIR +"/component/common/network/websocket/wsclient_tls.c>",
    # network lwip
    "+<" + SDK_DIR +"/component/common/network/dhcp/dhcps.c>",
    "+<" + SDK_DIR +"/component/common/network/sntp/sntp.c>",
    # network polarssl polarssl
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/aesni.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/blowfish.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/camellia.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/ccm.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/certs.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/cipher.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/cipher_wrap.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/debug.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/ecp_ram.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/entropy.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/entropy_poll.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/error.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/gcm.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/havege.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/md2.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/md4.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/memory_buffer_alloc.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/net.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/padlock.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/pbkdf2.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/pkcs11.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/pkcs12.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/pkcs5.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/pkparse.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/platform.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/ripemd160.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/ssl_cache.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/ssl_ciphersuites.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/ssl_cli.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/ssl_srv.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/ssl_tls.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/threading.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/timing.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/version.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/version_features.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/x509.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/x509_create.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/x509_crl.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/x509_crt.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/x509_csr.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/x509write_crt.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/x509write_csr.c>",
    "+<" + SDK_DIR +"/component/common/network/ssl/polarssl-1.3.8/library/xtea.c>",
    # network polarssl ssl_ram_map
    "+<" + SDK_DIR +"/component/common/network/ssl/ssl_ram_map/ssl_ram_map.c>",
    # os freertos portable
    "+<" + SDK_DIR +"/component/os/freertos/freertos_v8.1.2/Source/portable/MemMang/heap_5.c>",
    "+<" + SDK_DIR +"/component/os/freertos/freertos_v8.1.2/Source/portable/GCC/ARM_CM4F/port.c>",
    # "+<" + SDK_DIR +"/component/os/freertos/freertos_v8.1.2/Source/portable/IAR/ARM_CM4F/portasm.s>",
    # os freertos
    "+<" + SDK_DIR +"/component/os/freertos/cmsis_os.c>",
    "+<" + SDK_DIR +"/component/os/freertos/freertos_v8.1.2/Source/croutine.c>",
    "+<" + SDK_DIR +"/component/os/freertos/freertos_v8.1.2/Source/event_groups.c>",
    "+<" + SDK_DIR +"/component/os/freertos/freertos_service.c>",
    "+<" + SDK_DIR +"/component/os/freertos/freertos_v8.1.2/Source/list.c>",
    "+<" + SDK_DIR +"/component/os/freertos/freertos_v8.1.2/Source/queue.c>",
    "+<" + SDK_DIR +"/component/os/freertos/freertos_v8.1.2/Source/tasks.c>",
    "+<" + SDK_DIR +"/component/os/freertos/freertos_v8.1.2/Source/timers.c>",
    # os osdep
    "+<" + SDK_DIR +"/component/os/os_dep/device_lock.c>",
    "+<" + SDK_DIR +"/component/os/os_dep/osdep_service.c>",
    # peripheral api
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/analogin_api.c>",
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/dma_api.c>",
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/efuse_api.c>",
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/flash_api.c>",
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/gpio_api.c>",
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/gpio_irq_api.c>",
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/i2c_api.c>",
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/i2s_api.c>",
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/nfc_api.c>",
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/pinmap.c>",
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/pinmap_common.c>",
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/port_api.c>",
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/pwmout_api.c>",
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/rtc_api.c>",
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/serial_api.c>",
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/sleep.c>",
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/spi_api.c>",
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/sys_api.c>",
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/timer_api.c>",
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/us_ticker.c>",
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/us_ticker_api.c>",
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/wait_api.c>",
    "+<" + SDK_DIR +"/component/common/mbed/targets/hal/rtl8711b/wdt_api.c>",
    # peripheral rtl8710b
    "+<" + SDK_DIR +"/component/soc/realtek/8711b/fwlib/ram_lib/rtl8710b_dsleepcfg.c>",
    "+<" + SDK_DIR +"/component/soc/realtek/8711b/fwlib/ram_lib/rtl8710b_dstandbycfg.c>",
    "+<" + SDK_DIR +"/component/soc/realtek/8711b/fwlib/ram_lib/rtl8710b_intfcfg.c>",
    "+<" + SDK_DIR +"/component/soc/realtek/8711b/misc/rtl8710b_ota.c>",
    "+<" + SDK_DIR +"/component/soc/realtek/8711b/fwlib/ram_lib/rtl8710b_pinmapcfg.c>",
    "+<" + SDK_DIR +"/component/soc/realtek/8711b/fwlib/ram_lib/rtl8710b_sleepcfg.c>",
    # network - httpc
    "+<" + SDK_DIR +"/component/common/network/httpc/httpc_tls.c>",
    # network - httpd
    "+<" + SDK_DIR +"/component/common/network/httpd/httpd_tls.c>",
    # utilities
    "+<" + SDK_DIR +"/component/common/utilities/cJSON.c>",
    "+<" + SDK_DIR +"/component/common/utilities/http_client.c>",
    "+<" + SDK_DIR +"/component/common/utilities/uart_socket.c>",
    "+<" + SDK_DIR +"/component/common/utilities/xml.c>",
    # fmt: on
]

# Fixups
env.Append(CPPPATH=[FIXUPS_DIR])
sources += [
    "+<" + FIXUPS_DIR + "/app_start_patch.c>",
    "+<" + FIXUPS_DIR + "/cmsis_ipsr.c>",
    "+<" + FIXUPS_DIR + "/log_uart.c>",
    "+<" + FIXUPS_DIR + "/wifi_mode.c>",
]

# Libs & linker config
env.Append(
    LIBPATH=[
        # fmt: off
        join(SDK_DIR, "component", "soc", "realtek", "8711b", "misc", "bsp", "lib", "common", "GCC"),
        # fmt: on
        # linker script path
        join(PLATFORM_DIR, "ld"),
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
        join(PLATFORM_DIR, "ld", ldscript),
    ],
)

# Misc options
env.Replace(
    SIZEPROGREGEXP=r"^(?:\.ram_image2\.entry|\.ram_image2\.text|\.ram_image2\.data|\.xip_image2\.text)\s+([0-9]+).*",
    SIZEDATAREGEXP=r"^(?:\.ram_image2\.entry|\.ram_image2\.data|\.ram_image2\.bss|\.ram_image2\.skb\.bss)\s+([0-9]+).*",
    SIZECHECKCMD="$SIZETOOL -A -d $SOURCES",
    SIZEPRINTCMD="$SIZETOOL -B -d $SOURCES",
)

# Image conversion
def pick_tool(target, source, env):
    sections = [
        "__ram_image2_text_start__",
        "__ram_image2_text_end__",
        "__xip_image2_start__",
    ]
    addrs = [None] * len(sections)
    with open(env.subst("${BUILD_DIR}/${PROGNAME}.nmap")) as f:
        for line in f:
            for i, section in enumerate(sections):
                if section not in line:
                    continue
                addrs[i] = line.split()[0]
    files = [
        join("$BUILD_DIR", "ram_2.r.bin"),  # RAM image with padding
        join("$BUILD_DIR", "ram_2.bin"),  # RAM image, stripped
        join("$BUILD_DIR", "ram_2.p.bin"),  # RAM image, stripped, with header
        join("$BUILD_DIR", "xip_image2.bin"),  # raw firmware image
        join("$BUILD_DIR", "xip_image2.p.bin"),  # firmware with header
    ]
    commands = [
        f"$PICK 0x{addrs[0]} 0x{addrs[1]} {files[0]} {files[1]} raw",
        f"$PICK 0x{addrs[0]} 0x{addrs[1]} {files[1]} {files[2]}",
        f"$PICK 0x{addrs[2]} 0x{addrs[2]} {files[3]} {files[4]}",
    ]
    for command in commands:
        status = env.Execute("@" + command + " > " + join("$BUILD_DIR", "pick.txt"))
        if status:
            return status


def concat_xip_ram(target, source, env):
    with open(env.subst("${BUILD_DIR}/xip_image2.p.bin"), "rb") as f:
        xip = f.read()
    with open(env.subst("${BUILD_DIR}/ram_2.p.bin"), "rb") as f:
        ram = f.read()
    with open(env.subst("${BUILD_DIR}/${IMG_FW}"), "wb") as f:
        f.write(xip)
        f.write(ram)


def checksum_img(target, source, env):
    source = join("$BUILD_DIR", "$IMG_FW")
    status = env.Execute(f"@$CHECKSUM {source}")
    if status:
        return status


def package_ota(target, source, env):
    source = join("$BUILD_DIR", "$IMG_FW")
    target = join("$BUILD_DIR", "$IMG_OTA")
    status = env.Execute(
        f"@$OTA {source} {ota1_offset} {source} {ota2_offset} 0x20170111 {target}"
    )
    if status:
        return status


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
commands = [
    (
        "${PROGNAME}.nmap",
        [
            "$NM",
            "$SOURCE",
            "> $BIN",
        ],
    ),
    (
        "ram_2.r.bin",
        [
            "$OBJCOPY",
            "-j .ram_image2.entry",
            "-j .ram_image2.data",
            "-j .ram_image2.bss",
            "-j .ram_image2.skb.bss",
            "-j .ram_heap.data",
            "-O binary",
            "$SOURCE",
            "$BIN",
        ],
    ),
    (
        "xip_image2.bin",
        [
            "$OBJCOPY",
            "-j .xip_image2.text",
            "-O binary",
            "$SOURCE",
            "$BIN",
        ],
    ),
    (
        "rdp.bin",
        [
            "$OBJCOPY",
            "-j .ram_rdp.text",
            "-O binary",
            "$SOURCE",
            "$BIN",
        ],
    ),
]
actions = [
    env.VerboseAction(
        " ".join(command).replace("$BIN", join("$BUILD_DIR", target)),
        f"Generating {target}",
    )
    for target, command in commands
]
actions.append(env.VerboseAction(pick_tool, "Wrapping binary images"))
actions.append(env.VerboseAction(concat_xip_ram, "Packaging firmware image - $IMG_FW"))
# actions.append(env.VerboseAction(checksum_img, "Generating checksum"))
actions.append(env.VerboseAction(package_ota, "Packaging OTA image - $IMG_OTA"))
actions.append(env.VerboseAction("true", f"- OTA1 flash offset: {ota1_offset}"))
actions.append(env.VerboseAction("true", f"- OTA2 flash offset: {ota2_offset}"))

# Clone env to ignore options from child projects
envsdk = env.Clone()

# SDK library target
target_sdk = envsdk.BuildLibrary(
    join("$BUILD_DIR", "ambz_sdk"),
    SDK_DIR,
    sources,
)
target_boot = env.StaticLibrary(
    join("$BUILD_DIR", "boot_all"),
    env.BinToObj(
        join("$BUILD_DIR", "boot_all.o"),
        join(PLATFORM_DIR, "bin", boot_all),
    ),
)
env.Prepend(LIBS=[target_sdk, target_boot])

# Main firmware binary builder
env.Append(
    BUILDERS=dict(
        DumpFirmwareBinary=Builder(action=actions),
    ),
)

# Copyright (c) Kuba Szczodrzyński 2022-05-18.

from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()
platform = env.PioPlatform()


def env_add_lwip(
    env,
    version: str,
    port: str,
):
    # version = env["LIB_LWIP_VERSION"] if "LIB_LWIP_VERSION" in env else version_default
    package_dir = platform.get_package_dir(f"library-lwip@{version}-{port}")
    port_srcs = []
    port_includes = []

    if port in ["amb1"]:
        port_srcs = [
            "+<port/realtek/freertos/ethernetif.c>",
            "+<port/realtek/freertos/sys_arch.c>",
        ]
        port_includes = [
            "+<port/realtek>",
            "+<port/realtek/freertos>",
        ]
    elif port in ["bdk"]:
        port_srcs = [
            "+<port/*.c>",
        ]
        port_includes = [
            "+<port>",
        ]

    env.AddLibrary(
        name=f"lwip{version}_{port}",
        base_dir=package_dir,
        srcs=[
            "+<src/api/*.c>",
            "+<src/core/*.c>",
            "+<src/core/ipv4/*.c>",
            "+<src/netif/ethernet.c>",  # 2.0.x
            "+<src/netif/etharp.c>",  # 1.4.x
            "+<src/apps/mdns/mdns.c>",
            "+<src/apps/sntp/sntp.c>",
            *port_srcs,
        ],
        includes=[
            "+<src/include>",
            "+<src/include/ipv4>",
            *port_includes,
        ],
    )


env.AddMethod(env_add_lwip, "AddLibraryLwIP")

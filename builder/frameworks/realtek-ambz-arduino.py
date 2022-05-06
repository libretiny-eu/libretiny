# Copyright (c) Kuba Szczodrzyński 2022-04-22.

from os.path import join

from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()

env.Replace(AMBZ_NO_POLARSSL=True)

env.SConscript("realtek-ambz-sdk.py", exports="env")
env.SConscript("../arduino-common.py", exports="env")

# Flags
env.Append(
    CXXFLAGS=[
        # borrowed from RtlDuino/development/rtl87xx/platform.txt
        "-std=c++11",
        "-mcpu=cortex-m4",
        "-mthumb",
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16",
        "-g2",
        "-w",
        "-O2",
        "-MMD",
        "-fdata-sections",
        "-ffunction-sections",
        "-fmessage-length=0",
        "-fno-common",
        "-fno-exceptions",
        "-fno-rtti",
        "-fno-short-enums",
        "-fomit-frame-pointer",
        "-fsigned-char",
    ],
    CPPDEFINES=[
        ("ARDUINO", 10812),
        "ARDUINO_AMEBA",
        "ARDUINO_SDK",
        "ARDUINO_ARCH_AMBZ",
        "BOARD_${FAMILY}",
        # the SDK declares bool if not defined before
        # which conflicts with C++ built-in bool
        # so it's either -fpermissive or this:
        ("bool", "bool"),
        # enable LwIPRxBuffer
        "LT_HAS_LWIP",
        ("LT_PRINTF_BROKEN", "1"),  # printf does not handle %.3f properly
        "MBEDTLS_KEY_EXCHANGE__SOME__PSK_ENABLED",  # enable PSK in mbedTLS
        # "MBEDTLS_DEBUG_C",
    ],
    LINKFLAGS=[
        "--specs=nosys.specs",
        "-Wl,--as-needed",
        "-Wl,--build-id=none",
        "-Wl,--cref",
        "-Wl,--no-enum-size-warning",
        "-Wl,--no-undefined",
        "-Wl,--undefined=InfraStart",
        "-Wl,--warn-common",
    ],
)
# Arduino core uses __libc_init_array
env["LINKFLAGS"].remove("-nostartfiles")

# Sources - Arduino Core
env.AddLibrary(
    name="ambz_arduino_core",
    base_dir="$ARDUINO_DIR",
    srcs=[
        # Wiring core
        "+<cores/arduino/*.c>",
        "+<cores/arduino/*.cpp>",
    ],
    includes=[
        # prepend these as the Arduino core is incorrectly picking some includes from SDK
        "!<cores/arduino>",
        # includes that are missing in the vanilla SDK makefiles
        "+<$SDK_DIR/component/common/drivers/sdio/realtek/sdio_host/inc>",
        "+<$SDK_DIR/component/common/file_system/fatfs>",
        "+<$SDK_DIR/component/common/file_system/fatfs/r0.10c/include>",
        "+<$SDK_DIR/component/common/network/mdns>",
        "+<$SDK_DIR/component/common/network/libwsclient>",
    ],
)

# Sources - board variant
env.AddLibrary(
    name="ambz_${VARIANT}",
    base_dir="$BOARD_DIR",
    srcs=[
        "+<variant.cpp>",
    ],
)

# Sources - mbedTLS
env.AddLibrary(
    name="ambz_mbedtls",
    base_dir="$SDK_DIR",
    srcs=[
        # mbedTLS from SDK
        "+<component/common/network/ssl/mbedtls-2.4.0/library/*.c>",
        # skip ssl_tls.c for a fixup
        "-<component/common/network/ssl/mbedtls-2.4.0/library/ssl_tls.c>",
    ],
    includes=[
        "+<component/common/network/ssl/mbedtls-2.4.0/include>",
        # remove polarssl
        "-<component/common/network/ssl/polarssl-1.3.8/include>",
    ],
)

# Sources - platform fixups
env.AddLibrary(
    name="ambz_arduino_fixups",
    base_dir="$FIXUPS_DIR",
    srcs=[
        "+<ssl_tls.c>",  # rtl sdk defines S1 and S2 which conflicts here
    ],
)

# Sources - Arduino libraries
env.AddLibrary(
    name="ambz_arduino_libs",
    base_dir="$ARDUINO_DIR",
    srcs=[
        "+<libraries/*/*.cpp>",
    ],
    includes=[
        "+<libraries/*>",
    ],
)

# Libs & linker config
env.Append(
    LIBS=[
        "stdc++",
        "supc++",
    ],
)
env.Replace(
    LDSCRIPT_PATH=[
        join("$LD_DIR", "$LDSCRIPT_ARDUINO"),
    ],
)

# Build all libraries
env.BuildLibraries()

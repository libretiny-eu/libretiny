# Copyright (c) Etienne Le Cousin 2024-02-08.

from os.path import join

from platformio.platform.board import PlatformBoardConfig
from SCons.Script import DefaultEnvironment, Environment

env: Environment = DefaultEnvironment()
board: PlatformBoardConfig = env.BoardConfig()
queue = env.AddLibraryQueue("lightning-ln882h")
env.ConfigureFamily()

# Flags
queue.AppendPublic(
    CCFLAGS=[
        "-mcpu=cortex-m4",
        "-mthumb",
        "-mabi=aapcs",
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16",
        "-fno-short-enums",
        "-Wfatal-errors",
        "-ffunction-sections",
        "-fdata-sections",
        "-fno-strict-aliasing",
        "-MD","-MP",
    ],
    CFLAGS=[
        "-Wpointer-arith",
        "-Wno-write-strings",
        "-Wno-maybe-uninitialized",
    ],
    CXXFLAGS=[
    ],
    CPPDEFINES=[
        # other options
        "ARM_MATH_CM4",
        "LN882H",
        ("PLATFORM_LN882H","1"),
        ("MBEDTLS_CONFIG_FILE",r"\"mbedtls_config.h\""),
    ],
    ASFLAGS=[
        "-mcpu=cortex-m4",
        "-mthumb",
        "-mabi=aapcs",
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16",
        "-g",
        "-c",
    ],
    LINKFLAGS=[
        "-mcpu=cortex-m4",
        "-mthumb",
        "-mabi=aapcs",
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16",
        "--specs=nano.specs",
        # stdio wrappers (base/port/printf.c)
        "-Wl,-wrap,ln_printf",
        "-Wl,--gc-sections",
    ],
)
queue.AppendPrivate(
    CFLAGS=[
        "-Wno-implicit-function-declaration",
        "-Wno-incompatible-pointer-types",
        "-Wno-int-conversion",
        "-Wno-pointer-sign",
        "-Wfatal-errors",
        "-ffunction-sections",
        "-fdata-sections",
        "-fno-strict-aliasing",
        "-std=gnu99",
        "-MD","-MP",
    ],
)

# Boot files - from SDK project/wifi_mcu_basic_example
"""
queue.AddLibrary(
    name="ln882h_boot",
    base_dir="$SDK_DIR/project/wifi_mcu_basic_example",
    srcs=[
        "+<startup/startup_ln882h_gcc.c>",
        ],
)
"""
# Sources
queue.AddLibrary(
    name="ln882h_sdk",
    base_dir="$SDK_DIR",
    srcs=[
        # MCU
        "+<mcu/ln882h/system_ln882h.c>"
        # Hal
        "+<mcu/driver_ln882h/hal/*.c>",
        # FreeRTOS
        "+<components/kernel/FreeRTOS_Adapter/*.c>",
        # Serial
        "+<components/serial/serial.c>",
        # Misc
        "+<components/fs/nvds/ln_nvds.c>",
        "+<components/fs/kv/kv/ln_kv.c>",
        "+<components/fs/kv/kv_port/ln_kv_port.c>",
        "+<components/utils/fifo/fifobuf.c>",
        "+<components/utils/debug/log.c>",
        "+<components/utils/debug/CmBacktrace/cm_backtrace.c>",
        "+<components/utils/system_parameter.c>",
        "+<components/utils/power_mgmt/ln_pm.c>",
        "+<components/utils/runtime/runtime.c>",
        "+<components/utils/reboot_trace/reboot_trace.c>",
        "+<components/tencent/qcloud_iot_c_sdk/external_libs/cJSON/cJSON.c>",
    ],
    includes=[
        # MCU
        "+<mcu/CMSIS_5.3.0>",
        "+<mcu/ln882h>",
        # Hal
        "+<mcu/driver_ln882h>",
        "+<mcu/driver_ln882h/reg>",
        "+<mcu/driver_ln882h/hal>",
        # Components
        "+<components>",
        # FreeRTOS
        "+<components/kernel>",
        "+<components/kernel/FreeRTOS/Source/portable/GCC/ARM_CM4F>",
        "+<components/kernel/FreeRTOS_Adapter>",
        "+<components/kernel/osal>",
        # Misc
        "+<components/fs/nvds>",
        "+<components/fs/kv/kv>",
        "+<components/fs/kv/kv_port>",
        "+<components/serial>",
        "+<components/utils/debug/CmBacktrace>",
        "+<components/utils/power_mgmt>",
        #"+<components/utils/runtime>",
        "+<components/utils/reboot_trace>",
        "+<components/tencent/qcloud_iot_c_sdk/external_libs/cJSON>",
    ],
    options=dict(
        CPPDEFINES=["LN882H_SDK"],
        CFLAGS=["-w"],
    ),
)

queue.AddLibrary(
    name="ln882h_net",
    base_dir="$SDK_DIR",
    srcs=[
        # WiFi
        "+<components/wifi/wifi_manager/wifi_manager.c>",
        "+<components/wifi/wifi_lib_import/wifi_port.c>",
        # DHCPD
        "+<components/net/dhcpd/*.c>",
        # Utils
        "+<components/utils/ln_misc.c>",
        "+<components/utils/ln_sha1.c>",
        "+<components/utils/ln_aes.c>",
        "+<components/utils/ln_psk_calc.c>",
    ],
    includes=[
        "+<components/wifi/wifi_manager>",
        "+<components/wifi/wifi_lib_import>",
        "+<components/wifi/wifi_lib_export>",
        "+<components/net/dhcpd>",
    ],
    options=dict(
        CPPDEFINES=["LN882H_SDK"],
        CFLAGS=["-w"],
    ),
)


# Sources - FreeRTOS
env.Replace(FREERTOS_PORT=env["FAMILY_NAME"], FREERTOS_PORT_DEFINE="LIGHTNING_LN882H")
queue.AddExternalLibrary("freertos")
queue.AddExternalLibrary("freertos-port")

# Sources - lwIP
queue.AddExternalLibrary("lwip", port="ln882h")

# Sources - mbedTLS
queue.AddLibrary(
    name="ln882h_mbedtls",
    base_dir="$SDK_DIR",
    srcs=[
        # mbedTLS from SDK
        "+<components/net/mbedtls/library/*.c>",
        # replace this with a port specific
        "-<components/net/mbedtls/library/net_sockets.c>",
        "-<components/net/mbedtls/library/platform.c>",
        "-<components/net/mbedtls/library/threading_alt.c>",
        "-<components/net/mbedtls/library/timing.c>",
        # ln port
        "+<components/net/mbedtls/port_ln/library/*.c>",
    ],
    includes=[
        "+<components/net/mbedtls/include>",
        "+<components/net/mbedtls/port_ln/include>",
    ],
    options=dict(
        CPPDEFINES=["LN882H_SDK"],
        CFLAGS=["-w"],
    ),
)

# Libs & linker config
queue.AppendPublic(
    LIBPATH=[
        # fmt: off
        join("$SDK_DIR", "lib", "gcclib"),
        # fmt: on
    ],
    LIBS=[
        "arm_cortexM4lf_math",
        "ln882h_wifi",
        "m",
        "c",
        "nosys",
        "gcc",
   ],
)

# Generate linker scripts with correct flash offsets
env.GenerateLinkerScript(board, board.get("build.ldscript"))

# Build all libraries
queue.BuildLibraries()

# Main firmware outputs and actions
image_fw = "${BUILD_DIR}/image_firmware-ota-xz-v1.0.bin"
env.Replace(
    # linker command
    LINK='${LTCHIPTOOL} link2bin ${BOARD_JSON} "" ""',
    # UF2OTA input list
    UF2OTA=[
        # ota binary image for device
        f"{image_fw}=device:ota",
    ],
)

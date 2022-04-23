from os.path import isdir, join

from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()
platform = env.PioPlatform()
board = env.BoardConfig()

env.SConscript("realtek-ambz-sdk.py", exports="env")
env.SConscript("../arduino-api.py", exports="env")

mcu = board.get("build.mcu").upper()
family = board.get("build.family").upper()
variant = board.get("build.variant")
ldscript = board.get("build.ldscript_arduino")

SDK_DIR = platform.get_package_dir("framework-realtek-amb1")
BOARD_DIR = join(platform.get_dir(), "boards", variant)
FIXUPS_DIR = join(platform.get_dir(), "fixups", "realtek-ambz")
PLATFORM_DIR = join(platform.get_dir(), "platform", "realtek-ambz")
CORE_DIR = join(platform.get_dir(), "arduino", "realtek-ambz")
assert isdir(SDK_DIR)
assert isdir(env.subst(BOARD_DIR))
assert isdir(env.subst(FIXUPS_DIR))
assert isdir(env.subst(PLATFORM_DIR))
assert isdir(env.subst(CORE_DIR))

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
        f"BOARD_{family}",
        # the SDK declares bool and #ifndef somehow doesn't work...
        # so it's either -fpermissive or this:
        ("bool", "unsigned char"),
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

# Includes
env.Prepend(
    CPPPATH=[
        # fmt: off
        # prepend these as the Arduino core is incorrectly picking some includes from SDK
        join(CORE_DIR, "cores", "arduino"),
        join(CORE_DIR, "cores", "arduino", "avr"),
        join(CORE_DIR, "cores", "arduino", "rtl8195a"),
        join(CORE_DIR, "cores", "arduino", "spiffs"),
        # fmt: on
    ],
)
env.Append(
    CPPPATH=[
        # fmt: off
        # includes that are missing in the vanilla SDK makefiles
        join(SDK_DIR, "component", "common", "drivers", "sdio", "realtek", "sdio_host", "inc"),
        join(SDK_DIR, "component", "common", "file_system", "fatfs"),
        join(SDK_DIR, "component", "common", "file_system", "fatfs", "r0.10c", "include"),
        join(SDK_DIR, "component", "common", "network", "mdns"),
        join(SDK_DIR, "component", "common", "network", "libwsclient"),
        # fmt: on
    ],
)

# Sources
sources_core = [
    # fmt: off
    "+<" + CORE_DIR + "/cores/arduino/ard_socket.c>",
    "+<" + CORE_DIR + "/cores/arduino/avr/dtostrf.c>",
    "+<" + CORE_DIR + "/cores/arduino/b64.cpp>",
    "+<" + CORE_DIR + "/cores/arduino/cxxabi-compat.cpp>",
    "+<" + CORE_DIR + "/cores/arduino/hooks.c>",
    "+<" + CORE_DIR + "/cores/arduino/itoa.c>",
    "+<" + CORE_DIR + "/cores/arduino/LOGUARTClass.cpp>",
    "+<" + CORE_DIR + "/cores/arduino/lwip_info.c>",
    "+<" + CORE_DIR + "/cores/arduino/main.cpp>",
    "+<" + CORE_DIR + "/cores/arduino/PowerManagement.cpp>",
    "+<" + CORE_DIR + "/cores/arduino/RingBuffer.cpp>",
    "+<" + CORE_DIR + "/cores/arduino/rtl_sys.cpp>",
    "+<" + CORE_DIR + "/cores/arduino/server_drv.cpp>",
    "+<" + CORE_DIR + "/cores/arduino/spiffs/spiffs_cache.c>",
    "+<" + CORE_DIR + "/cores/arduino/spiffs/spiffs_check.c>",
    "+<" + CORE_DIR + "/cores/arduino/spiffs/spiffs_gc.c>",
    "+<" + CORE_DIR + "/cores/arduino/spiffs/spiffs_hydrogen.c>",
    "+<" + CORE_DIR + "/cores/arduino/spiffs/spiffs_nucleus.c>",
    "+<" + CORE_DIR + "/cores/arduino/ssl_drv.cpp>",
    "+<" + CORE_DIR + "/cores/arduino/Tone.cpp>",
    "+<" + CORE_DIR + "/cores/arduino/WebSocketClient.cpp>",
    "+<" + CORE_DIR + "/cores/arduino/wifi_drv.cpp>",
    "+<" + CORE_DIR + "/cores/arduino/WInterrupts.c>",
    "+<" + CORE_DIR + "/cores/arduino/wiring.c>",
    "+<" + CORE_DIR + "/cores/arduino/wiring_analog.c>",
    "+<" + CORE_DIR + "/cores/arduino/wiring_digital.c>",
    "+<" + CORE_DIR + "/cores/arduino/wiring_os.c>",
    "+<" + CORE_DIR + "/cores/arduino/wiring_pulse.cpp>",
    "+<" + CORE_DIR + "/cores/arduino/wiring_shift.c>",
    "+<" + CORE_DIR + "/cores/arduino/wiring_watchdog.c>",
    "+<" + CORE_DIR + "/cores/arduino/WMath.cpp>",
    "+<" + BOARD_DIR + "/variant.cpp>",
    # fmt: on
]

# Libs & linker config
env.Append(
    LIBS=[
        "stdc++",
        "supc++",
    ],
)
env.Replace(
    LDSCRIPT_PATH=[
        join(PLATFORM_DIR, "ld", ldscript),
    ],
)

# Clone env to ignore options from child projects
envarduino = env.Clone()

# Arduino Core library target
target_lib = envarduino.BuildLibrary(
    join("$BUILD_DIR", "ambz_arduino_core"),
    CORE_DIR,
    sources_core,
)
env.Prepend(LIBS=[target_lib])

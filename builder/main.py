# Copyright (c) Kuba Szczodrzyński 2022-04-20.

from SCons.Script import Default, DefaultEnvironment

env = DefaultEnvironment()
board = env.BoardConfig()

# Utilities
env.SConscript("utils.py", exports="env")
# Vendor-specific library ports
env.SConscript("libs/lwip.py", exports="env")

# Firmware name
if env.get("PROGNAME", "program") == "program":
    env.Replace(PROGNAME="firmware")
env.Replace(PROGSUFFIX=".elf")

# Toolchain config - TODO multiple arch, specified in board.json
env.Replace(
    AR="arm-none-eabi-gcc-ar",
    AS="arm-none-eabi-gcc",
    CC="arm-none-eabi-gcc",
    CXX="arm-none-eabi-g++",
    GDB="arm-none-eabi-gdb",
    NM="arm-none-eabi-gcc-nm",
    LINK="arm-none-eabi-gcc",
    LD="arm-none-eabi-gcc",
    OBJCOPY="arm-none-eabi-objcopy",
    OBJDUMP="arm-none-eabi-objdump",
    # RANLIB="arm-none-eabi-gcc-ranlib",
    SIZETOOL="arm-none-eabi-size",
)

# Flash layout defines
flash_layout: dict = board.get("flash")
if flash_layout:
    defines = {}
    for name, layout in flash_layout.items():
        name = name.upper()
        (offset, _, length) = layout.partition("+")
        defines[f"FLASH_{name}_OFFSET"] = offset
        defines[f"FLASH_{name}_LENGTH"] = length
    env.Append(CPPDEFINES=defines.items())
    env.Replace(**defines)

# Platform builders details:
# - call env.AddDefaults("platform name", "sdk name") to add dir paths
# - call env.AddLibrary("lib name", "base dir", [sources]) to add lib sources
# - output main firmware image binary as $IMG_FW
# - call env.BuildLibraries() to build lib targets with safe envs
# - script code ordering:
#   - global vars
#   - # Outputs
#   - # Tools
#   - # Flags (C(XX)FLAGS / CPPDEFINES / LINKFLAGS)
#   - sources (env.AddLibrary)
#   - # Libs & linker config (LIBPATH / LIBS / LDSCRIPT_PATH)
#   - # Misc options
#   - # Image conversion (tools, functions, builders, actions, etc.)
#   - # Uploader
#   - # Library targets
#   - env.BuildLibraries()
#   - # Main firmware binary builder

target_elf = env.BuildProgram()
target_fw = env.DumpFirmwareBinary("$IMG_FW", target_elf)
env.AddPlatformTarget("upload", target_fw, env["UPLOAD_ACTIONS"], "Upload")
Default(target_fw)

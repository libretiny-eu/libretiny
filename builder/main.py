# Copyright (c) Kuba Szczodrzyński 2022-04-20.

import sys

from SCons.Script import Default, DefaultEnvironment

env = DefaultEnvironment()
board = env.BoardConfig()

# Utilities
env.SConscript("utils.py", exports="env")
env.SConscript("uf2.py", exports="env")
# Vendor-specific library ports
env.SConscript("libs/lwip.py", exports="env")
env.SConscript("libs/flashdb.py", exports="env")

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
    OBJCOPY="arm-none-eabi-objcopy",
    OBJDUMP="arm-none-eabi-objdump",
    # RANLIB="arm-none-eabi-gcc-ranlib",
    SIZETOOL="arm-none-eabi-size",
)

# Flash layout defines
flash_layout: dict = board.get("flash")
if flash_layout:
    defines = {}
    flash_size = 0
    fal_items = ""
    for name, layout in flash_layout.items():
        name = name.upper()
        (offset, _, length) = layout.partition("+")
        defines[f"FLASH_{name}_OFFSET"] = offset
        defines[f"FLASH_{name}_LENGTH"] = length
        fal_items += f"FAL_PART_TABLE_ITEM({name.lower()}, {name})"
        flash_size = max(flash_size, int(offset, 16) + int(length, 16))
    defines["FLASH_LENGTH"] = flash_size
    defines["FAL_PART_TABLE"] = "{" + fal_items + "}"
    env.Append(CPPDEFINES=defines.items())
    env.Replace(**defines)

# Family builders details:
# - call env.AddDefaults("family name", "sdk name") to add dir paths
# - call env.AddLibrary("lib name", "base dir", [sources]) to add lib sources
# - call env.BuildLibraries() to build lib targets with safe envs
# - configure LINK, UF2OTA and UPLOAD_ACTIONS
# - script code ordering:
#   - global vars
#   - # Tools
#   - # Flags (C(XX)FLAGS / CPPDEFINES / LINKFLAGS)
#   - sources (env.AddLibrary)
#   - # Libs & linker config (LIBPATH / LIBS / LDSCRIPT_PATH)
#   - # Misc options
#   - # Uploader
#   - # Bootloader library
#   - env.BuildLibraries()
#   - # Main firmware outputs and actions

target_elf = env.BuildProgram()
targets = [target_elf]

if "UF2OTA" in env:
    target_uf2 = env.BuildUF2OTA(target_elf)
    targets.append(target_uf2)
    env.AddUF2Uploader(target_uf2)
elif "IMG_FW" in env:
    target_fw = env.subst("$IMG_FW")
    env.AddPlatformTarget("upload", target_fw, env["UPLOAD_ACTIONS"], "Upload")
else:
    sys.stderr.write("Warning! Firmware outputs not specified.\n")

Default(targets)

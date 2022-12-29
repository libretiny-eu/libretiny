# Copyright (c) Kuba Szczodrzyński 2022-04-20.

import sys

from SCons.Script import Default, DefaultEnvironment

env = DefaultEnvironment()
platform = env.PioPlatform()
board = env.BoardConfig()

# Make tools available
sys.path.insert(0, platform.get_dir())

# Utilities
env.SConscript("utils/config.py", exports="env")
env.SConscript("utils/env.py", exports="env")
env.SConscript("utils/flash.py", exports="env")
env.SConscript("utils/libs.py", exports="env")
env.SConscript("utils/ltchiptool.py", exports="env")
# Vendor-specific library ports
env.SConscript("libs/flashdb.py", exports="env")
env.SConscript("libs/lwip.py", exports="env")
env.SConscript("libs/printf.py", exports="env")

# Firmware name
if env.get("PROGNAME", "program") == "program":
    env.Replace(PROGNAME="firmware")
env.Replace(PROGSUFFIX=".elf")

prefix = board.get("build.prefix", "")
env.Replace(
    AR=prefix + "gcc-ar",
    AS=prefix + "gcc",
    CC=prefix + "gcc",
    CXX=prefix + "g++",
    GDB=prefix + "gdb",
    NM=prefix + "gcc-nm",
    LINK=prefix + "gcc",
    OBJCOPY=prefix + "objcopy",
    OBJDUMP=prefix + "objdump",
    RANLIB=prefix + "gcc-ranlib",
    SIZETOOL=prefix + "size",
)

# Default environment options
env.AddDefaults(platform, board)
# Flash layout defines
env.AddFlashLayout(board)

# Family builders details:
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
    env.AddFlashWriter(target_uf2)
elif "IMG_FW" in env:
    target_fw = env.subst("$IMG_FW")
    env.AddPlatformTarget("upload", target_fw, env["UPLOAD_ACTIONS"], "Upload")
else:
    sys.stderr.write(
        "Warning! Firmware outputs not specified. Uploading is not possible.\n"
    )

Default(targets)

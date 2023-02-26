# Copyright (c) Kuba Szczodrzyński 2022-04-20.

import sys

from platformio.platform.board import PlatformBoardConfig
from SCons.Script import Default, DefaultEnvironment, Environment

env: Environment = DefaultEnvironment()
board: PlatformBoardConfig = env.BoardConfig()

# Utilities
env.SConscript("utils/config.py", exports="env")
env.SConscript("utils/cores.py", exports="env")
env.SConscript("utils/env.py", exports="env")
env.SConscript("utils/flash.py", exports="env")
env.SConscript("utils/libs-external.py", exports="env")
env.SConscript("utils/libs.py", exports="env")
env.SConscript("utils/ltchiptool.py", exports="env")

# Firmware name
if env.get("PROGNAME", "program") == "program":
    env.Replace(PROGNAME="firmware")
env.Replace(PROGSUFFIX=".elf")

# Configure the toolchain
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

# Framework builder (base.py/arduino.py) is executed in BuildProgram()
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

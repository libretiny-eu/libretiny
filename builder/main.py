# Copyright (c) Kuba Szczodrzyński 2022-04-20.

import sys
from os.path import join

from platformio.platform.base import PlatformBase
from platformio.platform.board import PlatformBoardConfig
from SCons.Script import (
    COMMAND_LINE_TARGETS,
    AlwaysBuild,
    Default,
    DefaultEnvironment,
    Environment,
)

env: Environment = DefaultEnvironment()
platform: PlatformBase = env.PioPlatform()
board: PlatformBoardConfig = env.BoardConfig()

python_deps = {
    "ltchiptool": ">=4.12.2,<5.0",
}
env.SConscript("python-venv.py", exports="env")
env.ConfigurePythonVenv()
env.InstallPythonDependencies(python_deps)

# Utilities
env.SConscript("utils/config.py", exports="env")
env.SConscript("utils/cores.py", exports="env")
env.SConscript("utils/env.py", exports="env")
env.SConscript("utils/flash.py", exports="env")
env.SConscript("utils/libs-external.py", exports="env")
env.SConscript("utils/libs-queue.py", exports="env")
env.SConscript("utils/ltchiptool-util.py", exports="env")

# Firmware name
if env.get("PROGNAME", "program") == "program":
    env.Replace(PROGNAME="raw_firmware")
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

# Environment variables, include paths, etc.
env.ConfigureEnvironment(platform, board)

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
# Force including the base framework in case no other is specified
if "nobuild" not in COMMAND_LINE_TARGETS and not env.get("PIOFRAMEWORK"):
    env.SConscript("frameworks/base.py")

#
# Target: Build executable and linkable firmware
#
target_uf2 = join("${BUILD_DIR}", "firmware.uf2")
if "nobuild" in COMMAND_LINE_TARGETS:
    target_elf = join("${BUILD_DIR}", "${PROGNAME}.elf")
    env["UF2OTA"] = "dummy"  # forcefully allow uploading using ltchiptool
else:
    target_elf = env.BuildProgram()
    target_uf2 = env.BuildUF2OTA(target_uf2, target_elf)
    env.Depends(target_uf2, "checkprogsize")

AlwaysBuild(env.Alias("nobuild", target_uf2))
target_buildprog = env.Alias("buildprog", target_uf2, target_uf2)

#
# Target: Print binary size
#
target_size = env.Alias(
    "size",
    target_elf,
    env.VerboseAction("${SIZEPRINTCMD}", "Calculating size ${SOURCE}"),
)
AlwaysBuild(target_size)

#
# Target: Upload firmware
#
upload_protocol = env.subst("${UPLOAD_PROTOCOL}") or "uart"
upload_actions = []
upload_source = target_uf2
ltchiptool_flags = "UF2OTA" in env and env.GetLtchiptoolWriteFlags()

if ltchiptool_flags:
    # use ltchiptool for flashing, if available
    env.Replace(
        LTCHIPTOOL_FLAGS=ltchiptool_flags,
        UPLOADER="${LTCHIPTOOL} flash write",
        UPLOADCMD="${UPLOADER} ${LTCHIPTOOL_FLAGS} ${UPLOADERFLAGS} ${SOURCE}",
    )
    upload_actions = [
        env.VerboseAction(env.AutodetectUploadPort, "Looking for upload port..."),
        env.VerboseAction("${UPLOADCMD}", "Uploading ${SOURCE}"),
    ]
elif upload_protocol == "custom":
    upload_actions = [env.VerboseAction("$UPLOADCMD", "Uploading $SOURCE")]
else:
    sys.stderr.write("Warning! Unknown upload protocol %s\n" % upload_protocol)

AlwaysBuild(env.Alias("upload", upload_source, upload_actions))

#
# Default targets
#
Default([target_buildprog, target_size])

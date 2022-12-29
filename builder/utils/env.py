# Copyright (c) Kuba Szczodrzyński 2022-05-04.

import sys
from os.path import isdir, join
from subprocess import PIPE, Popen

from ltchiptool import Family
from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()


def read_version(platform_dir: str, version: str):
    if not isdir(join(platform_dir, ".git")):
        sys.stderr.write("Warning! Non-Git installations are NOT SUPPORTED.\n")
        return version
    try:
        p = Popen(
            ["git", "rev-parse", "--short", "HEAD"], stdout=PIPE, cwd=platform_dir
        )
        if p.wait() != 0:
            sys.stderr.write(
                f"Warning! Non-zero return code received from Git: {p.returncode}\n"
            )
            return version
        sha = p.stdout.read().decode().strip()

        p = Popen(["git", "status", "--short"], stdout=PIPE, cwd=platform_dir)
        if p.wait() != 0:
            sys.stderr.write(
                f"Warning! Non-zero return code received from Git: {p.returncode}\n"
            )
            return version
        dirty = p.stdout.read().strip()
    except (FileNotFoundError, IndexError):
        sys.stderr.write(
            "Warning! Git executable not found, or unreadable data received. Cannot read version information.\n"
        )
        return version

    ids = [
        sha and "sha",
        sha[:7] or None,
        "dirty" if dirty else None,
    ]
    build_str = ".".join(filter(None, ids))
    return f"{version}+{build_str}" if build_str else version


def env_add_defaults(env, platform, board):
    # Get Family object for this board
    family = Family.get(short_name=board.get("build.family"))
    # Default environment variables
    vars = dict(
        SDK_DIR=platform.get_package_dir(family.framework),
        LT_DIR=platform.get_dir(),
        # Root dirs
        BOARD_DIR=join("${LT_DIR}", "boards", "${VARIANT}"),
        ARDUINO_DIRS={
            "$FAMILY_CODE": join("${LT_DIR}", "arduino", "${FAMILY_NAME}"),
            "$FAMILY_PARENT_CODE": join("${LT_DIR}", "arduino", "${FAMILY_PARENT}"),
        },
        FAMILY_DIR=join("${LT_DIR}", "platform", "${FAMILY_NAME}"),
        PARENT_DIR=join("${LT_DIR}", "platform", "${FAMILY_PARENT}"),
        TOOLS_DIR=join("${LT_DIR}", "tools"),
        # Family-specific paths
        BIN_DIR=join("${FAMILY_DIR}", "bin"),
        OPENOCD_DIR=join("${FAMILY_DIR}", "openocd"),
        # Use SDK linker script by default
        LDSCRIPT_PATH=["${LDSCRIPT_SDK}"],
        # Board config variables
        MCU=board.get("build.mcu").upper(),
        MCULC=board.get("build.mcu"),
        VARIANT=board.get("build.variant"),
        LDSCRIPT_SDK=board.get("build.ldscript_sdk"),
        LDSCRIPT_ARDUINO=board.get("build.ldscript_arduino"),
        # ltchiptool config:
        # -r    output raw log messages
        # -i 1  indent log messages
        LTCHIPTOOL='"${PYTHONEXE}" -m ltchiptool -r -i 1',
        # Fix for link2bin to get tmpfile name in argv
        LINKCOM="${LINK} ${LINKARGS}",
        LINKARGS="${TEMPFILE('-o $TARGET $LINKFLAGS $__RPATH $SOURCES $_LIBDIRFLAGS $_LIBFLAGS', '$LINKCOMSTR')}",
    )
    env.Replace(**vars)
    # Store family parameters as environment variables
    env.Replace(**dict(family))
    # Default build options
    env.Prepend(
        CPPPATH=[
            "$LT_DIR/platform/common/config",
            "$LT_DIR/platform/common/fixups",
            "$LT_DIR/platform/common/fixups/lib_inc",
            "$BOARD_DIR",
            "$FAMILY_DIR/config",
            "$PARENT_DIR/config",
            "$FAMILY_DIR/fixups",
            "$PARENT_DIR/fixups",
            "$FAMILY_DIR/fixups/inc",
            "$PARENT_DIR/fixups/inc",
        ],
        LIBPATH=[
            "$FAMILY_DIR/ld",
            "$PARENT_DIR/ld",
            "$FAMILY_DIR/fixups",
            "$PARENT_DIR/fixups",
        ],
        CPPDEFINES=[
            ("LIBRETUYA", "1"),
            ("LT_VERSION", read_version(platform.get_dir(), platform.version)),
            ("LT_BOARD", "${VARIANT}"),
            ("F_CPU", board.get("build.f_cpu")),
            ("MCU", "${MCU}"),
            ("FAMILY", "F_${FAMILY}"),
        ],
        LINKFLAGS=[
            '"-Wl,-Map=' + join("$BUILD_DIR", "${PROGNAME}.map") + '"',
        ],
    )


env.AddMethod(env_add_defaults, "AddDefaults")

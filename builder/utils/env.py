# Copyright (c) Kuba Szczodrzyński 2022-05-04.

from os.path import join

from SCons.Script import DefaultEnvironment

from tools.util.platform import get_family

env = DefaultEnvironment()


def env_add_defaults(env, platform, board):
    # Get Family object for this board
    family = get_family(short_name=board.get("build.family"))
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
        # Link2Bin tool
        LINK2BIN='"${PYTHONEXE}" "${LT_DIR}/tools/link2bin.py"',
        UF2OTA_PY='"${PYTHONEXE}" "${LT_DIR}/tools/uf2ota/uf2ota.py"',
        UF2UPLOAD_PY='"${PYTHONEXE}" "${LT_DIR}/tools/upload/uf2upload.py"',
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
            ("LT_VERSION", platform.version),
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

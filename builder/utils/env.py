# Copyright (c) Kuba Szczodrzyński 2022-05-04.

from os.path import isdir, join

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
        VARIANT=board.get("build.variant"),
        LDSCRIPT_SDK=board.get("build.ldscript_sdk"),
        LDSCRIPT_ARDUINO=board.get("build.ldscript_arduino"),
        # Link2Bin tool
        LINK2BIN='"${PYTHONEXE}" "${LT_DIR}/tools/link2bin.py"',
        UF2OTA_PY='"${PYTHONEXE}" "${LT_DIR}/tools/uf2ota/uf2ota.py"',
        UF2UPLOAD_PY='"${PYTHONEXE}" "${LT_DIR}/tools/upload/uf2upload.py"',
    )
    env.Replace(**vars)
    # Store family parameters as environment variables
    env.Replace(**dict(family))
    # Check directories for validity
    for k, v in vars.items():
        if k.endswith("_DIR"):
            assert isdir(env.subst(v)), f"{env.subst(v)} is not a directory"
    # Default build options
    env.Prepend(
        CPPPATH=[
            "$BOARD_DIR",
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
            ("FAMILY", "${FAMILY}"),
        ],
    )


env.AddMethod(env_add_defaults, "AddDefaults")

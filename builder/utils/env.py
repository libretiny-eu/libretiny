# Copyright (c) Kuba Szczodrzyński 2022-05-04.

import json
import sys
from os import makedirs
from os.path import isdir, join
from subprocess import PIPE, Popen
from typing import Dict

from ltchiptool import Family, get_version
from ltchiptool.util.lvm import LVM
from ltchiptool.util.misc import sizeof
from platformio.platform.base import PlatformBase
from platformio.platform.board import PlatformBoardConfig
from SCons.Script import DefaultEnvironment, Environment

env: Environment = DefaultEnvironment()


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

        p = Popen(["git", "diff", "--quiet"], stdout=PIPE, cwd=platform_dir)
        dirty = p.wait() != 0
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


def env_configure(
    env: Environment,
    platform: PlatformBase,
    board: PlatformBoardConfig,
) -> Family:
    # Read external libraries list
    with open(join(platform.get_dir(), "external-libs.json")) as f:
        external_libs = json.load(f)
    # Get Family object for this board
    family = Family.get(short_name=board.get("build.family"))
    # Default environment variables
    env.Replace(
        SDK_DIR=platform.get_package_dir(board.get("package")),
        LT_DIR=platform.get_dir(),
        CORES_DIR=join("${LT_DIR}", "cores"),
        COMMON_DIR=join("${LT_DIR}", "cores", "common"),
        LT_VERSION=read_version(platform.get_dir(), platform.version),
        # Build directories & paths
        VARIANTS_DIR=join("${LT_DIR}", "boards", "variants"),
        FAMILY_DIR=join("${LT_DIR}", "cores", "${FAMILY_NAME}"),
        MISC_DIR=join("${FAMILY_DIR}", "misc"),
        LDSCRIPT_PATH=[board.get("build.ldscript")],
        # Board config variables
        MCU=board.get("build.mcu").upper(),
        MCULC=board.get("build.mcu").lower(),
        VARIANT=board.get("build.variant"),
        # ltchiptool config:
        # -r    output raw log messages
        # -i 1  indent log messages
        LTCHIPTOOL='"${LTPYTHONEXE}" -m ltchiptool -r -i 1 -L "${LT_DIR}"',
        # Fix for link2bin to get tmpfile name in argv
        LINKCOM="${LINK} ${LINKARGS}",
        LINKARGS="${TEMPFILE('-o $TARGET $LINKFLAGS $__RPATH $SOURCES $_LIBDIRFLAGS $_LIBFLAGS', '$LINKCOMSTR')}",
        # Store the family object
        FAMILY_OBJ=family,
        EXTERNAL_LIBS=external_libs,
    )
    # Store family parameters as environment variables
    env.Replace(**dict(family))
    # Set platform directory in ltchiptool (for use in this process only)
    LVM.add_path(platform.get_dir())
    return family


def env_print_info(
    env: Environment,
    platform: PlatformBase,
    board: PlatformBoardConfig,
):
    TAB = " " * 4

    def dump(k, v, indent=""):
        k = k.replace("#", ".")
        if isinstance(v, dict):
            print(f"{indent} - {k}:")
            for k, v in sorted(v.items()):
                dump(k, v, indent + TAB)
        elif isinstance(v, list):
            print(f"{indent} - {k}:")
            for k, v in enumerate(v):
                dump(k, v, indent + TAB)
        else:
            print(f"{indent} - {k} = {v}")

    # Print information about installed core versions
    print("PLATFORM VERSIONS:")
    print(" - libretiny @", env["LT_VERSION"])
    print(" - ltchiptool @", get_version())
    # Print custom platformio.ini options
    if platform.custom_opts:
        print("CUSTOM OPTIONS:")
        for k, v in sorted(platform.custom_opts.items()):
            dump(k, v)
    # Print custom flash layout
    if env.get("FLASH_IS_CUSTOM", False):
        print("CUSTOM FLASH LAYOUT:")
        for name, layout in board.get("flash").items():
            (_, _, length) = layout.partition("+")
            length = int(length, 16)
            print(f" - {name}: {layout} ({sizeof(length)})")


def env_parse_custom_options(env: Environment, platform: PlatformBase):
    opts: dict = platform.custom_opts.get("options", None)
    if not opts:
        return
    headers = {
        "lwip": "lwipopts.h",
        "freertos": "FreeRTOSConfig.h",
    }
    for header, options in list(opts.items()):
        if not isinstance(options, str):
            raise TypeError("Options value should be str")
        options = options.strip().splitlines()
        opts_dict = {}
        for line in options:
            if "=" not in line:
                raise ValueError(f"Invalid option: {line}")
            k, _, v = line.partition("=")
            k = k.strip()
            v = v.strip()
            opts_dict[k] = v
        # replace predefined header names
        opts.pop(header)
        header = headers.get(header, header)
        header = header.replace(".", "#")
        opts[header] = opts_dict


def env_apply_custom_options(env: Environment, platform: PlatformBase):
    opts = platform.custom_opts.get("options", None)
    if not opts:
        return
    header_dir = join("${BUILD_DIR}", "include")
    real_dir = env.subst(header_dir)
    makedirs(real_dir, exist_ok=True)

    for header, options in opts.items():
        header: str
        options: Dict[str, str]
        # open the header file for writing
        header = header.replace("#", ".")
        f = open(join(real_dir, header), "w")
        f.write(f'#include_next "{header}"\n' "\n" "#pragma once\n" "\n")
        # write all #defines
        for k, v in options.items():
            f.write(
                f"// {k} = {v}\n"
                f"#ifdef {k}\n"
                f"#undef {k}\n"
                f"#endif\n"
                f"#define {k} {v}\n"
            )
        f.close()
    # prepend newly created headers before any other
    env.Prepend(CPPPATH=[header_dir])


env.AddMethod(env_configure, "ConfigureEnvironment")
env.AddMethod(env_print_info, "PrintInfo")
env.AddMethod(env_parse_custom_options, "ParseCustomOptions")
env.AddMethod(env_apply_custom_options, "ApplyCustomOptions")

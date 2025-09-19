# Copyright (c) Kuba Szczodrzyński 2022-06-12.

import re
from os.path import isfile, join
from typing import Dict

from ltchiptool.util.fileio import chext
from platformio.platform.base import PlatformBase
from platformio.platform.board import PlatformBoardConfig
from SCons.Script import DefaultEnvironment, Environment

env: Environment = DefaultEnvironment()


def env_parse_custom_flash_layout(
    env: Environment,
    platform: PlatformBase,
    board: PlatformBoardConfig,
):
    opts: dict = platform.custom_opts.get("flash", None)
    if not opts:
        return
    flash_layout: dict = board.get("flash")

    # find all default partitions
    partitions: Dict[str, int] = {}
    flash_size = 0
    for name, layout in flash_layout.items():
        (offset, _, length) = layout.partition("+")
        offset = int(offset, 16)
        length = int(length, 16)
        partitions[name] = offset
        flash_size = max(flash_size, offset + length)

    # set custom offsets
    for name, offset in opts.items():
        offset = int(offset, 0)
        partitions[name] = offset

    # recalculate partition sizes
    flash_layout = {}
    partitions = sorted(partitions.items(), key=lambda p: p[1])
    for i, (name, offset) in enumerate(partitions):
        end = partitions[i + 1][1] if i + 1 < len(partitions) else flash_size
        length = end - offset
        flash_layout[name] = f"0x{offset:06X}+0x{length:X}"
    board.manifest["flash"] = flash_layout
    env["FLASH_IS_CUSTOM"] = True


def env_add_flash_layout(env: Environment, board: PlatformBoardConfig):
    flash_layout: dict = board.get("flash")
    if flash_layout:
        defines = {}
        flash_size = 0
        fal_items = ""
        # add "root" partition
        fal_items += "FAL_PART_TABLE_ITEM(root,ROOT)"
        # add all partitions
        for name, layout in flash_layout.items():
            name = name.upper()
            (offset, _, length) = layout.partition("+")
            offset = int(offset, 16)
            length = int(length, 16)
            defines[f"FLASH_{name}_OFFSET"] = f"0x{offset:06X}"
            defines[f"FLASH_{name}_LENGTH"] = f"0x{length:06X}"
            fal_items += f"FAL_PART_TABLE_ITEM({name.lower()}, {name})"
            flash_size = max(flash_size, offset + length)
        defines["FLASH_LENGTH"] = f"0x{flash_size:06X}"
        # for "root" partition
        defines["FLASH_ROOT_OFFSET"] = "0x000000"
        defines["FLASH_ROOT_LENGTH"] = f"0x{flash_size:06X}"
        # add partition table array
        defines["FAL_PART_TABLE"] = "{" + fal_items + "}"
        env.Replace(FLASH_DEFINES=defines)
        env.Replace(**defines)


def env_generate_linker_script(env: Environment, board: PlatformBoardConfig, name: str):
    template_name = chext(name, "template.ld")

    # find the linker script template in LIBPATH
    input = None
    for path in env["LIBPATH"]:
        path = env.subst(path)
        if isfile(join(path, template_name)):
            input = join(path, template_name)
            break
    if not input:
        raise FileNotFoundError(template_name)

    # load the .template.ld script
    with open(input, "r") as f:
        ldscript = f.read()

    def transform(match: re.Match):
        key = match[1]
        if key in env:
            return env[key]
        if key.startswith("BOARD_"):
            key = key[6:].lower()
            return board.get(key)
        raise ValueError(f"Unrecognized template key: {key}")

    ldscript = re.sub(r"\${([A-Z0-9_.]+)}", transform, ldscript)

    # write .ld script
    output = join("${BUILD_DIR}", name)
    with open(env.subst(output), "w") as f:
        f.write(ldscript)

    env.Prepend(LIBPATH=["${BUILD_DIR}"])


env.AddMethod(env_parse_custom_flash_layout, "ParseCustomFlashLayout")
env.AddMethod(env_add_flash_layout, "AddFlashLayout")
env.AddMethod(env_generate_linker_script, "GenerateLinkerScript")

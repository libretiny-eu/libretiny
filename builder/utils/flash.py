# Copyright (c) Kuba Szczodrzyński 2022-06-12.

from SCons.Script import DefaultEnvironment, Environment

env: Environment = DefaultEnvironment()


def env_add_flash_layout(env: Environment, board):
    flash_layout: dict = board.get("flash")
    if flash_layout:
        defines = {}
        flash_size = 0
        fal_items = ""
        # add "root" partition
        fal_items += "FAL_PART_TABLE_ITEM(root, ROOT)"
        # add all partitions
        for name, layout in flash_layout.items():
            name = name.upper()
            (offset, _, length) = layout.partition("+")
            defines[f"FLASH_{name}_OFFSET"] = offset
            defines[f"FLASH_{name}_LENGTH"] = length
            fal_items += f"FAL_PART_TABLE_ITEM({name.lower()}, {name})"
            flash_size = max(flash_size, int(offset, 16) + int(length, 16))
        defines["FLASH_LENGTH"] = flash_size
        # for "root" partition
        defines["FLASH_ROOT_OFFSET"] = 0
        defines["FLASH_ROOT_LENGTH"] = flash_size
        # add partition table array
        defines["FAL_PART_TABLE"] = "{" + fal_items + "}"
        env.Append(CPPDEFINES=list(defines.items()))
        env.Replace(**defines)


env.AddMethod(env_add_flash_layout, "AddFlashLayout")

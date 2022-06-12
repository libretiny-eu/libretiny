# Copyright (c) Kuba Szczodrzyński 2022-06-12.

from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()


def env_add_flash_layout(env, board):
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
        env.Append(CPPDEFINES=list(defines.items()))
        env.Replace(**defines)


env.AddMethod(env_add_flash_layout, "AddFlashLayout")

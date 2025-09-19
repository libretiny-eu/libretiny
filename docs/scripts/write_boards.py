# Copyright (c) Kuba Szczodrzyński 2022-05-31.

import os
import sys

while os.getcwd() in sys.path:
    sys.path.remove(os.getcwd())

import re
from os.path import dirname, isfile, join

import colorama
from colorama import Fore, Style
from ltchiptool import Board, Family
from ltchiptool.util.fileio import readjson, readtext
from ltchiptool.util.misc import sizeof
from markdown import Markdown

OUTPUT = join(dirname(__file__), "..", "status")


def load_chip_type_h() -> str:
    code = readtext(
        join(
            dirname(__file__),
            "..",
            "..",
            "cores",
            "common",
            "base",
            "lt_types.h",
        )
    )
    code = re.sub(r"//.+", "", code)
    code = re.sub(r"/\*.+\*/", "", code)
    return code


def parse_enum(code: str, name: str) -> dict[str, str]:
    code = code.replace("\t", " ")
    code = code.partition(f"{name};")[0]
    code = code.rpartition("{")[2]
    code = code.strip().strip("{}").strip()
    code = [line.strip().strip(",").strip() for line in code.split("\n")]
    code = filter(None, code)
    code = [line.partition("=") for line in code]
    code = {key.strip(): value.strip() for key, _, value in code}
    return code


def get_families_json() -> dict[str, int]:
    return {
        family.short_name: family.id for family in Family.get_all() if family.is_chip
    }


def get_mcus_boards(boards: list[Board], aliases: dict[str, str]) -> dict[str, str]:
    out = {}

    def check_mcu(mcu_name, family_name):
        if mcu_name in out and out[mcu_name] != family_name:
            print(
                Fore.RED
                + f"ERROR: MCU '{mcu_name}' of board '{board.name}' belongs to multiple families: '{out[mcu_name]}' and '{family_name}'"
                + Style.RESET_ALL,
                file=sys.stderr,
            )
        out[mcu_name] = family_name

    for board in boards:
        mcu_name: str = board["build.mcu"].upper()
        mcu_alias: str = board["doc.mcu"]
        family_name: str = board.family.short_name
        check_mcu(mcu_name, family_name)
        if mcu_alias:
            mcu_alias = mcu_alias.upper()
            check_mcu(mcu_alias, family_name)
            if mcu_alias not in aliases:
                print(
                    Fore.RED
                    + f"ERROR: MCU alias '{mcu_alias}' of board '{board.name}' is not defined in enum"
                    + Style.RESET_ALL,
                    file=sys.stderr,
                )
            elif aliases[mcu_alias] != mcu_name:
                print(
                    Fore.RED
                    + f"ERROR: MCU alias '{mcu_alias}' of board '{board.name}' doesn't match real name '{mcu_name}'"
                    + Style.RESET_ALL,
                    file=sys.stderr,
                )
    return out


def get_families_enum(code: str) -> dict[str, int]:
    return {
        family[2:]: int(family_id, 16)
        for family, family_id in parse_enum(code, "lt_cpu_family_t").items()
    }


def get_mcus_enum(code: str) -> tuple[dict[str, str], dict[str, str]]:
    mcus = {}
    aliases = {}
    enum = parse_enum(code, "lt_cpu_model_t")
    for mcu, mcu_id in enum.items():
        while mcu_id in enum:
            aliases[mcu] = mcu_id
            mcu_id = enum[mcu_id]
        mcus[mcu] = mcu_id.split("(")[1].split(",")[0][2:]
    return mcus, aliases


def get_readme_family_link(family: Family) -> str | None:
    for f in family.inheritance[::-1]:
        path = f"../platform/{f.name}/README.md"
        if isfile(join(dirname(__file__), path)):
            return path
    return None


def get_readme_board_link(board: Board) -> str:
    return f"../../boards/{board.name}/README.md"


def board_json_sort(tpl):
    return tpl[1]["mcu"], tpl[0]


def board_obj_sort(board: Board):
    generic = board.is_generic
    vendor = board.vendor
    if vendor == "N/A":
        vendor = "\xff"
        generic = False
    return (
        not generic,  # reverse
        vendor,
        board["build.mcu"],
        board["mcu"],
        board.name,
    )


def get_board_symbol(board: Board) -> str:
    return board.symbol or board.generic_name or board.name.upper()


def write_chips(mcus: dict[str, Family], aliases: dict[str, str]):
    md = Markdown(OUTPUT, "supported_chips")
    chips = []
    clones = []
    for mcu, family in sorted(mcus.items()):
        docs = get_readme_family_link(family)
        target = chips
        if mcu in aliases:
            mcu = f"{mcu} ({aliases[mcu]})"
            target = clones
        if docs:
            target.append(md.get_link(mcu, docs))
        else:
            target.append(mcu)
    md.add_list(*chips, *clones)
    md.write()


def write_boards(boards: list[Board]):
    md = Markdown(OUTPUT, "supported_boards")
    header = [
        "Name",
        "MCU",
        "Flash",
        "RAM",
        "Pins*",
        "Wi-Fi",
        "BLE",
        "ZigBee",
        "Family name",
    ]
    rows = []

    vendor_prev = ""
    for board in boards:
        # add board vendor as a row
        vendor = board["vendor"]
        if vendor_prev != vendor:
            rows.append([f"**{vendor}**"])
            vendor_prev = vendor
        # count total pin count & IO count
        pins = "-"
        pinout: dict[str, dict] = board["pcb.pinout"]
        if pinout:
            pinout = [pin for name, pin in pinout.items() if name.isnumeric()]
            pins_total = len(pinout)
            pins_io = sum(1 for pin in pinout if "ARD" in pin)
            pins = f"{pins_total} ({pins_io} I/O)"
        # format row values
        symbol = get_board_symbol(board)
        docs = get_readme_board_link(board)
        board_url = f"[{symbol}]({docs})"
        row = [
            board_url,
            board["build.mcu"].upper(),
            sizeof(board["upload.flash_size"]),
            sizeof(board["upload.maximum_ram_size"]),
            pins,
            "✔️" if "wifi" in board["connectivity"] else "❌",
            "✔️" if "ble" in board["connectivity"] else "❌",
            "✔️" if "zigbee" in board["connectivity"] else "❌",
            f"`{board.family.name}`",
        ]
        rows.append(row)
    md.add_table(header, *rows)
    md.write()


def write_unsupported_boards(
    series: dict[str, dict[str, dict]],
    name: str,
    supported: list[str],
    mcus: dict[str, Family],
    generics: dict[str, list[Board]],
):
    md = Markdown(OUTPUT, name)
    header = [
        "Name",
        "MCU",
        "Flash",
        "RAM",
        "Pins",
        "Wi-Fi",
        "BLE",
        "ZigBee",
    ]
    rows = []
    for series_name in sorted(series):
        series_rows = []
        series_rows.append([f"**{series_name.upper()} Series**"])
        boards = series[series_name]
        for board_name, board in sorted(boards.items(), key=board_json_sort):
            if board_name in supported:
                continue
            board_text = board_name.upper()
            mcu_text = mcu = board["mcu"].upper()
            if mcu in generics:
                generic = generics[mcu][0]
                board_text += f' :material-information-outline:{{ title="You can use {generic.name} board instead" }}'
            if mcu in mcus:
                docs = get_readme_family_link(mcus[mcu])
                if docs:
                    mcu_text = md.get_link(mcu, docs)
            row = [
                board_text,
                mcu_text,
                sizeof(board["flash"]) if board["flash"] else "?",
                sizeof(board["ram"]) if board["ram"] else "?",
                str(board["pins_total"]),
                "✔️" if "wifi" in board["connectivity"] else "❌",
                "✔️" if "ble" in board["connectivity"] else "❌",
                "✔️" if "zigbee" in board["connectivity"] else "❌",
            ]
            series_rows.append(row)
        if series_rows:
            rows += series_rows
    md.add_table(header, *rows)
    md.write()


def write_families(supported: list[Family]):
    md = Markdown(OUTPUT, "supported_families")
    header = [
        "Title",
        "Name",
        "Code",
        "Short name & ID",
        "Supported?",
        "Source SDK",
    ]
    rows = []

    for family in Family.get_all():
        # TODO update the table to support parent-child relationship
        if not family.is_chip:
            continue
        docs = get_readme_family_link(family)
        row = [
            # Title
            (
                "[{}]({})".format(
                    family.description,
                    docs,
                )
                if docs
                else family.description
            ),
            # Name
            family.is_supported and f"`{family.name}`" or "`-`",
            # Code
            family.is_supported and f"`{family.code}`" or "`-`",
            # Short name & ID
            "`{}` (0x{:X})".format(
                family.short_name,
                family.id,
            ),
            # Arduino Core
            (
                "✔️"
                if family in supported
                and family.is_supported
                and family.has_arduino_core
                else "❌"
            ),
            # Source SDK
            (
                "[`{}`]({})".format(
                    family.target_package,
                    f"https://github.com/libretiny-eu/{family.target_package}",
                )
                if family.target_package
                else "-"
            ),
        ]
        rows.append(row)
    md.add_table(header, *rows)
    md.write()


def write_boards_list(boards: list[Board]):
    md = Markdown(join(dirname(__file__), ".."), join("..", "boards", "SUMMARY"))
    items = []
    for board in boards:
        symbol = get_board_symbol(board)
        if board.is_generic:
            symbol = board["name"]
        items.append(f"[{symbol}](../boards/{board.name}/README.md)")
    md.add_list(*items)
    md.write()


if __name__ == "__main__":
    colorama.init()

    boards = map(Board, Board.get_list())
    boards = sorted(boards, key=board_obj_sort)
    code = load_chip_type_h()

    errors = False

    for board in boards:
        if board.name != board["source"]:
            print(
                Fore.RED
                + f"ERROR: Invalid build.variant of '{board['source']}': '{board.name}'"
                + Style.RESET_ALL,
                file=sys.stderr,
            )
            errors = True

    families_json = get_families_json()
    families_enum = get_families_enum(code)
    families_json_keys = set(families_json.keys())
    families_enum_keys = set(families_enum.keys())
    mcus_enum, mcu_aliases = get_mcus_enum(code)
    mcus_boards = get_mcus_boards(boards, mcu_aliases)
    mcus_boards_keys = set(mcus_boards.keys())
    mcus_enum_keys = set(mcus_enum.keys())
    mcus_missing_in_boards = mcus_enum_keys - mcus_boards_keys
    mcus_missing_in_enum = mcus_boards_keys - mcus_enum_keys

    # check if all families are defined in lt_types.h and families.json
    if families_json_keys != families_enum_keys:
        print(
            Fore.RED + f"ERROR: Inconsistent lt_types.h vs families.json:",
            file=sys.stderr,
        )
        print(
            "- Missing in JSON: " + ", ".join(families_enum_keys - families_json_keys),
            file=sys.stderr,
        )
        print(
            "- Missing in enum: " + ", ".join(families_json_keys - families_enum_keys),
            file=sys.stderr,
        )
        print(Style.RESET_ALL, end="", file=sys.stderr)
        errors = True

    # verify that family IDs match
    for family in families_json_keys.union(families_enum_keys):
        if (
            family in families_json
            and family in families_enum
            and families_json[family] != families_enum[family]
        ):
            print(
                Fore.RED
                + f"ERROR: Family ID mismatch for '{family}': 0x{families_json[family]:08X} vs 0x{families_enum[family]:08X}"
                + Style.RESET_ALL,
                file=sys.stderr,
            )
            errors = True

    # warn if any enum MCUs are unused in boards
    if mcus_missing_in_boards:
        print(
            Fore.YELLOW
            + f"NOTICE: Unused MCUs: "
            + ", ".join(mcus_missing_in_boards)
            + Style.RESET_ALL
        )

    # fail if any board MCUs are undefined in enum
    if mcus_missing_in_enum:
        print(
            Fore.RED
            + f"ERROR: Undefined MCUs in lt_types.h: "
            + ", ".join(mcus_missing_in_enum)
            + Style.RESET_ALL,
            file=sys.stderr,
        )
        errors = True

    # check if all MCUs belong to the correct family
    for mcu in mcus_boards_keys.union(mcus_enum_keys):
        if (
            mcu in mcus_boards
            and mcu in mcus_enum
            and mcus_boards[mcu] != mcus_enum[mcu]
        ):
            print(
                Fore.RED
                + f"ERROR: MCU family mismatch for '{mcu}': '{mcus_boards[mcu]}' vs '{mcus_enum[mcu]}'"
                + Style.RESET_ALL,
                file=sys.stderr,
            )
            errors = True

    if errors:
        exit(1)

    # check all supported families by MCU presence in enum
    families_supported = sorted(families_enum_keys.intersection(mcus_enum.values()))
    families_supported = [Family.get(f) for f in families_supported]
    # filter out MCUs of unsupported families
    mcus_boards.update(mcus_enum)
    mcus_all = {}
    for mcu, family in mcus_boards.items():
        family = Family.get(family)
        if family not in families_supported:
            continue
        mcus_all[mcu] = family
    # remove boards of unsupported families
    boards = [board for board in boards if board.family in families_supported]

    # find generic variants of boards
    generics: dict[str, list[Board]] = {}
    for board in boards:
        if not board.is_generic:
            continue
        mcu: str = board["build.mcu"].upper()
        if mcu not in generics:
            generics[mcu] = []
        generics[mcu].append(board)

    write_chips(mcus_all, mcu_aliases)
    write_boards(boards)
    write_boards_list(boards)
    write_families(families_supported)

    board_lists = [
        "boards_tuya_all",
    ]
    for name in board_lists:
        file = join(dirname(__file__), "..", f"{name}.json")
        data = readjson(file)
        write_unsupported_boards(
            series=data,
            name=f"unsupported_{name}",
            supported=[board.name for board in boards],
            mcus=mcus_all,
            generics=generics,
        )

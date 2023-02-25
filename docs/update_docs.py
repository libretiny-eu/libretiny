# Copyright (c) Kuba Szczodrzyński 2022-05-31.

import sys
from os.path import dirname, isfile, join

sys.path.append(join(dirname(__file__), ".."))

import re
from typing import Dict, List, Set

import colorama
from colorama import Fore, Style
from ltchiptool import Board, Family
from ltchiptool.util.fileio import readjson, readtext
from ltchiptool.util.misc import sizeof
from markdown import Markdown

OUTPUT = join(dirname(__file__), "status")


def load_chip_type_h() -> str:
    code = readtext(
        join(
            dirname(__file__),
            "..",
            "cores",
            "common",
            "base",
            "lt_chip.h",
        )
    )
    code = re.sub(r"//.+", "", code)
    code = re.sub(r"/\*.+\*/", "", code)
    return code


def check_mcus(boards: List[Board]) -> bool:
    for board in boards:
        # check if all boards' MCUs are defined in families.json
        mcu_name: str = board["build.mcu"]
        mcus = [mcu.lower() for mcu in board.family.mcus]
        if mcu_name not in mcus:
            print(
                Fore.RED
                + f"ERROR: MCU '{mcu_name}' of board '{board.name}' is not defined for family '{board.family.name}'"
                + Style.RESET_ALL
            )
            return False
    return True


def get_family_mcus() -> Set[str]:
    out = []
    for family in Family.get_all():
        out += family.mcus
    return set(out)


def get_family_names() -> Set[str]:
    return set(family.short_name for family in Family.get_all() if family.is_chip)


def get_board_mcus(boards: List[Board]) -> Set[str]:
    out = set()
    for board in boards:
        mcu_name: str = board["build.mcu"]
        out.add(mcu_name.upper())
    return out


def get_enum_keys(code: str, name: str) -> Set[str]:
    code = code.replace("\t", " ")
    code = code.partition(f"{name};")[0]
    code = code.rpartition("{")[2]
    code = code.strip().strip("{}").strip()
    code = [line.strip().strip(",").strip() for line in code.split("\n")]
    code = filter(None, code)
    return set(line.partition(" ")[0].strip() for line in code)


def get_enum_mcus(code: str) -> Set[str]:
    return get_enum_keys(code, "ChipType")


def get_enum_families(code: str) -> Set[str]:
    return set(family[2:] for family in get_enum_keys(code, "ChipFamily"))


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


def write_chips(mcus: List[str]):
    md = Markdown(OUTPUT, "supported_chips")
    md.add_list(*mcus)
    md.write()


def write_boards(boards: List[Board]):
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
        pinout: Dict[str, dict] = board["pcb.pinout"]
        if pinout:
            pinout = [pin for name, pin in pinout.items() if name.isnumeric()]
            pins_total = len(pinout)
            pins_io = sum(1 for pin in pinout if "ARD" in pin)
            pins = f"{pins_total} ({pins_io} I/O)"
        # format row values
        symbol = get_board_symbol(board)
        board_url = f"[{symbol}](../../boards/{board.name}/README.md)"
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
    series: Dict[str, Dict[str, dict]],
    name: str,
    supported: List[str],
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
            row = [
                board_name.upper(),
                board["mcu"].upper(),
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


def write_families():
    md = Markdown(OUTPUT, "supported_families")
    header = [
        "Title",
        "Name (parent)",
        "Code",
        "Short name & ID",
        "Arduino Core",
        "Source SDK",
    ]
    rows = []

    for family in Family.get_all():
        # TODO update the table to support parent-child relationship
        if not family.is_chip:
            continue
        docs = None
        for f in family.inheritance:
            readme = join(dirname(__file__), "platform", f.name, "README.md")
            if isfile(readme):
                docs = f"../{f.name}/"
        row = [
            # Title
            "[{}]({})".format(
                family.description,
                docs,
            )
            if docs
            else family.description,
            # Name (parent)
            family.is_supported
            and (
                f"`{family.name}`"
                if not family.parent
                else f"`{family.name}` (`{family.parent_name}`)"
            )
            or "`-`",
            # Code
            family.is_supported
            and (
                f"`{family.code}`"
                if not family.parent
                else f"`{family.code}` (`{family.parent_code}`)"
            )
            or "`-`",
            # Short name & ID
            "`{}` (0x{:X})".format(
                family.short_name,
                family.id,
            ),
            # Arduino Core
            "✔️" if family.is_supported and family.has_arduino_core else "❌",
            # Source SDK
            "[`{}`]({})".format(
                family.target_package,
                f"https://github.com/libretuya/{family.target_package}",
            )
            if family.target_package
            else "-",
        ]
        rows.append(row)
    md.add_table(header, *rows)
    md.write()


def write_boards_list(boards: List[Board]):
    md = Markdown(dirname(__file__), join("..", "boards", "SUMMARY"))
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
                + Style.RESET_ALL
            )
            errors = True

    families_json = get_family_names()
    families_enum = get_enum_families(code)
    if families_json != families_enum:
        print(Fore.RED + f"ERROR: Inconsistent JSON families vs ChipType.h families:")
        print("- Missing in JSON: " + ", ".join(families_enum - families_json))
        print("- Missing in enum: " + ", ".join(families_json - families_enum))
        print(Style.RESET_ALL, end="")
        errors = True

    mcus_json = get_family_mcus()
    mcus_enum = get_enum_mcus(code)
    mcus_boards = get_board_mcus(boards)
    if mcus_json != mcus_enum:
        print(
            Fore.YELLOW + f"NOTICE: Inconsistent JSON family MCUs vs ChipType.h MCUs:"
        )
        print("- Missing in JSON: " + ", ".join(mcus_enum - mcus_json))
        print("- Missing in enum: " + ", ".join(mcus_json - mcus_enum))
        print(Style.RESET_ALL, end="")
        # this is not considered an error (for now)
        # errors = True

    if not check_mcus(boards):
        errors = True

    if errors:
        exit(1)

    write_chips(sorted(mcus_boards.union(mcus_json)))
    write_boards(boards)
    write_boards_list(boards)
    write_families()

    boards_all = [
        "boards_tuya_all",
    ]
    for name in boards_all:
        file = join(dirname(__file__), f"{name}.json")
        data = readjson(file)
        write_unsupported_boards(
            series=data,
            name=f"unsupported_{name}",
            supported=[board.name for board in boards],
        )

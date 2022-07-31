# Copyright (c) Kuba Szczodrzyński 2022-05-31.

import sys
from os.path import dirname, join

sys.path.append(join(dirname(__file__), ".."))
import re
from typing import Dict, List, Set, Tuple

import colorama
from colorama import Fore, Style

from tools.util.fileio import readjson, readtext
from tools.util.markdown import Markdown
from tools.util.obj import get, sizeof
from tools.util.platform import (
    get_board_list,
    get_board_manifest,
    get_families,
    get_family,
)

OUTPUT = join(dirname(__file__), "status")


def load_boards() -> Dict[str, dict]:
    return {board: get_board_manifest(board) for board in get_board_list()}


def load_chip_type_h() -> str:
    code = readtext(
        join(
            dirname(__file__),
            "..",
            "arduino",
            "libretuya",
            "core",
            "ChipType.h",
        )
    )
    code = re.sub(r"//.+", "", code)
    code = re.sub(r"/\*.+\*/", "", code)
    return code


def check_mcus(boards: List[Tuple[str, dict]]) -> bool:
    for board_name, board in boards:
        # check if all boards' MCUs are defined in families.json
        family_name: str = get(board, "build.family")
        mcu_name: str = get(board, "build.mcu")
        family = get_family(short_name=family_name)
        if not family:
            print(
                Fore.RED
                + f"ERROR: Family '{family_name}' of board '{board_name}' does not exist"
                + Style.RESET_ALL
            )
            return False
        mcus = [mcu.lower() for mcu in family.mcus]
        if mcu_name not in mcus:
            print(
                Fore.RED
                + f"ERROR: MCU '{mcu_name}' of board '{board_name}' is not defined for family '{family_name}'"
                + Style.RESET_ALL
            )
            return False
    return True


def get_family_mcus() -> Set[str]:
    out = []
    for family in get_families():
        out += family.mcus
    return set(out)


def get_family_names() -> Set[str]:
    return set(family.short_name for family in get_families())


def get_board_mcus(boards: List[Tuple[str, dict]]) -> Set[str]:
    out = set()
    for _, board in boards:
        mcu_name: str = get(board, "build.mcu")
        out.add(mcu_name.upper())
    return out


def get_enum_keys(code: str, name: str) -> Set[str]:
    code = code.partition(f"enum {name}")[2]
    code = code.partition(";")[0]
    code = code.strip().strip("{}").strip()
    code = [line.strip().strip(",").strip() for line in code.split("\n")]
    code = filter(None, code)
    return set(line.partition(" ")[0].strip() for line in code)


def get_enum_mcus(code: str) -> Set[str]:
    return get_enum_keys(code, "ChipType")


def get_enum_families(code: str) -> Set[str]:
    return set(family[2:] for family in get_enum_keys(code, "ChipFamily"))


def board_sort(tpl):
    generic = tpl[0].lower().startswith("generic")
    vendor = get(tpl[1], "vendor")
    if vendor == "N/A":
        vendor = "\xff"
        generic = False
    return (
        not generic,  # reverse
        vendor,
        get(tpl[1], "build.mcu"),
        get(tpl[1], "mcu"),
        tpl[0],
    )


def get_board_symbol(board_name: str, board: dict) -> str:
    symbol = get(board, "symbol")
    if not symbol and board_name.startswith("generic-"):
        symbol = board_name[8:]
    else:
        symbol = symbol or board_name.upper()
    return symbol


def write_chips(mcus: List[str]):
    md = Markdown(OUTPUT, "supported_chips")
    md.add_list(*mcus)
    md.write()


def write_boards(boards: List[Tuple[str, dict]]):
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
    for board_name, board in boards:
        family = get_family(short_name=get(board, "build.family"))
        # add board vendor as a row
        vendor = get(board, "vendor")
        if vendor_prev != vendor:
            rows.append([f"**{vendor}**"])
            vendor_prev = vendor
        # count total pin count & IO count
        pins = "-"
        pinout: Dict[str, dict] = get(board, "pcb.pinout")
        if pinout:
            pinout = [pin for name, pin in pinout.items() if name.isnumeric()]
            pins_total = len(pinout)
            pins_io = sum(1 for pin in pinout if "ARD" in pin)
            pins = f"{pins_total} ({pins_io} I/O)"
        # format row values
        symbol = get_board_symbol(board_name, board)
        board_url = f"[{symbol}](../../boards/{board_name}/README.md)"
        row = [
            board_url,
            get(board, "build.mcu").upper(),
            sizeof(get(board, "upload.flash_size")),
            sizeof(get(board, "upload.maximum_ram_size")),
            pins,
            "✔️" if "wifi" in get(board, "connectivity") else "❌",
            "✔️" if "ble" in get(board, "connectivity") else "❌",
            "✔️" if "zigbee" in get(board, "connectivity") else "❌",
            f"`{family.name}`",
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
        for board_name, board in sorted(boards.items(), key=board_sort):
            if board_name in supported:
                continue
            row = [
                board_name.upper(),
                board["mcu"].upper(),
                sizeof(board["flash"]),
                sizeof(board["ram"]),
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

    for family in get_families():
        row = [
            # Title
            "[{}]({})".format(
                family.description,
                family.url,
            )
            if family.url
            else family.description,
            # Name (parent)
            f"`{family.name or '-'}`"
            if not family.parent
            else f"`{family.name}` (`{family.parent}`)",
            # Code
            f"`{family.code or '-'}`"
            if not family.parent
            else f"`{family.code}` (`{family.parent_code}`)",
            # Short name & ID
            "`{}` (0x{:X})".format(
                family.short_name,
                family.id,
            ),
            # Arduino Core
            "✔️" if family.has_arduino_core else "❌",
            # Source SDK
            "`{}` ([{}]({}))".format(
                family.framework,
                family.sdk_name,
                family.sdk,
            )
            if family.sdk
            else "-",
        ]
        rows.append(row)
    md.add_table(header, *rows)
    md.write()


def write_boards_list(boards: List[Tuple[str, dict]]):
    md = Markdown(dirname(__file__), join("..", "boards", "SUMMARY"))
    items = []
    for board_name, board in boards:
        symbol = get_board_symbol(board_name, board)
        if board_name.startswith("generic-"):
            symbol = get(board, "name")
        items.append(f"[{symbol}](../boards/{board_name}/README.md)")
    md.add_list(*items)
    md.write()


if __name__ == "__main__":
    colorama.init()

    boards = load_boards()
    boards = sorted(boards.items(), key=board_sort)
    code = load_chip_type_h()

    errors = False

    for name, board in boards:
        variant = get(board, "build.variant")
        if name != variant:
            print(
                Fore.RED
                + f"ERROR: Invalid build.variant of '{name}': '{variant}'"
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
            supported=[tpl[0] for tpl in boards],
        )

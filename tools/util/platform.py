# Copyright (c) Kuba Szczodrzyński 2022-06-02.

from glob import glob
from os.path import basename, dirname, isfile, join
from typing import Dict, List, Union

from tools.util.models import Family
from tools.util.obj import load_json, merge_dicts

boards_base: Dict[str, dict] = {}
families: List[Family] = []


def get_board_list() -> List[str]:
    boards_glob = join(dirname(__file__), "..", "..", "boards", "*.json")
    return [basename(file)[:-5] for file in glob(boards_glob)]


def get_board_manifest(board: Union[str, dict]) -> dict:
    boards_dir = join(dirname(__file__), "..", "..", "boards")
    if not isinstance(board, dict):
        if not isfile(board):
            board = join(boards_dir, f"{board}.json")
        board = load_json(board)
    if "_base" in board:
        base = board["_base"]
        if not isinstance(base, list):
            base = [base]
        result = {}
        for base_name in base:
            if base_name not in boards_base:
                file = join(boards_dir, "_base", f"{base_name}.json")
                boards_base[base_name] = load_json(file)
            merge_dicts(result, boards_base[base_name])
        merge_dicts(result, board)
        board = result
    return board


def get_families() -> List[Family]:
    global families
    if families:
        return families
    file = join(dirname(__file__), "..", "..", "families.json")
    families = [Family(f) for f in load_json(file)]
    return families


def get_family(
    any: str = None,
    id: Union[str, int] = None,
    short_name: str = None,
    name: str = None,
    code: str = None,
) -> Family:
    if any:
        id = any
        short_name = any
        name = any
        code = any
    if id and isinstance(id, str) and id.startswith("0x"):
        id = int(id, 16)
    for family in get_families():
        if id and family.id == id:
            return family
        if short_name and family.short_name == short_name.upper():
            return family
        if name and family.name == name.lower():
            return family
        if code and family.code == code.lower():
            return family
    if any:
        raise ValueError(f"Family not found - {any}")
    text = ", ".join(filter(None, [id, short_name, name, code]))
    raise ValueError(f"Family not found - {text}")

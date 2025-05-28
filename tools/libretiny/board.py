# Copyright (c) Kuba Szczodrzyński 2022-07-29.

from genericpath import isfile
from typing import Union

from .dict import merge_dicts
from .fileio import readjson
from .lvm import lvm_load_json


class Board:
    @staticmethod
    def get_data(board: Union[str, dict]) -> dict:
        if not isinstance(board, dict):
            if isfile(board):
                board = readjson(board)
                if not board:
                    raise FileNotFoundError(f"Board not found: {board}")
            else:
                source = board
                board = lvm_load_json(f"boards/{board}.json")
                board["source"] = source
        if "_base" in board:
            base = board["_base"]
            if not isinstance(base, list):
                base = [base]
            result = {}
            for base_name in base:
                board_base = lvm_load_json(f"boards/_base/{base_name}.json")
                merge_dicts(result, board_base)
            merge_dicts(result, board)
            board = result
        return board

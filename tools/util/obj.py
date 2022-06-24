# Copyright (c) Kuba Szczodrzyński 2022-06-02.

import json
from typing import Tuple, Union

SliceLike = Union[slice, str, int]


def merge_dicts(d1, d2):
    if d1 is not None and type(d1) != type(d2):
        raise TypeError("d1 and d2 are different types")
    if isinstance(d2, list):
        if d1 is None:
            d1 = []
        d1.extend(merge_dicts(None, item) for item in d2)
    elif isinstance(d2, dict):
        if d1 is None:
            d1 = {}
        for key in d2:
            d1[key] = merge_dicts(d1.get(key, None), d2[key])
    else:
        d1 = d2
    return d1


def load_json(file: str) -> Union[dict, list]:
    with open(file, "r", encoding="utf-8") as f:
        return json.load(f)


def get(data: dict, path: str):
    if not isinstance(data, dict) or not path:
        return None
    if "." not in path:
        return data.get(path, None)
    key, _, path = path.partition(".")
    return get(data.get(key, None), path)


def slice2int(val: SliceLike) -> Tuple[int, int]:
    """Convert a slice-like value (slice, string '7:0' or '3', int '3')
    to a tuple of (start, stop)."""
    if isinstance(val, int):
        return (val, val)
    if isinstance(val, slice):
        if val.step:
            raise ValueError("value must be a slice without step")
        if val.start < val.stop:
            raise ValueError("start must not be less than stop")
        return (val.start, val.stop)
    if isinstance(val, str):
        if ":" in val:
            val = val.split(":")
            if len(val) == 2:
                return tuple(map(int, val))
        elif val.isnumeric():
            return (int(val), int(val))
    raise ValueError(f"invalid slice format: {val}")


# https://stackoverflow.com/a/1094933/9438331
def sizeof(num: int, suffix="iB", base=1024.0) -> str:
    for unit in ["", "K", "M", "G", "T", "P", "E", "Z"]:
        if abs(num) < base:
            return f"{num:.1f} {unit}{suffix}".replace(".0 ", " ")
        num /= base
    return f"{num:.1f} Y{suffix}".replace(".0 ", " ")

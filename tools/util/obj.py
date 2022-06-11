# Copyright (c) Kuba Szczodrzyński 2022-06-02.

import json
from typing import Tuple, Union

SliceLike = Union[slice, str, int]


def merge_dicts(d1, d2, path=None):
    if path is None:
        path = []
    for key in d2:
        if key in d1 and isinstance(d1[key], dict) and isinstance(d2[key], dict):
            merge_dicts(d1[key], d2[key], path + [str(key)])
        else:
            d1[key] = d2[key]
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

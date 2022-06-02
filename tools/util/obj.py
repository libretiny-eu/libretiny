# Copyright (c) Kuba Szczodrzyński 2022-06-02.

import json
from typing import Union


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

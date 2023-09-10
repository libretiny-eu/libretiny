# Copyright (c) Kuba Szczodrzyński 2022-06-02.

from enum import Enum
from typing import Type

# The following helpers force using base dict class' methods.
# Because RecursiveDict uses these helpers, this prevents it
# from running into endless nested loops.


def get(data: dict, path: str):
    if not isinstance(data, dict) or not path:
        return None
    if dict.__contains__(data, path):
        return dict.get(data, path, None)
    key, _, path = path.partition(".")
    return get(dict.get(data, key, None), path)


def pop(data: dict, path: str, default=None):
    if not isinstance(data, dict) or not path:
        return default
    if dict.__contains__(data, path):
        return dict.pop(data, path, default)
    key, _, path = path.partition(".")
    return pop(dict.get(data, key, None), path, default)


def has(data: dict, path: str) -> bool:
    if not isinstance(data, dict) or not path:
        return False
    if dict.__contains__(data, path):
        return True
    key, _, path = path.partition(".")
    return has(dict.get(data, key, None), path)


def set_(data: dict, path: str, value, newtype=dict):
    if not isinstance(data, dict) or not path:
        return
    # can't use __contains__ here, as we're setting,
    # so it's obvious 'data' doesn't have the item
    if "." not in path:
        dict.__setitem__(data, path, value)
    else:
        key, _, path = path.partition(".")
        # allow creation of parent objects
        if key in data:
            sub_data = dict.__getitem__(data, key)
        else:
            sub_data = newtype()
            dict.__setitem__(data, key, sub_data)
        set_(sub_data, path, value)


def str2enum(cls: Type[Enum], key: str):
    if not key:
        return None
    try:
        return next(e for e in cls if e.name.lower() == key.lower())
    except StopIteration:
        return None

# Copyright (c) Kuba Szczodrzyński 2022-07-29.

from .obj import get, has, pop, set_


class RecursiveDict(dict):
    def __init__(self, data: dict = None):
        if data:
            data = {
                key: RecursiveDict(value) if isinstance(value, dict) else value
                for key, value in data.items()
            }
            super().__init__(data)
        else:
            super().__init__()

    def __getitem__(self, key):
        if "." not in key:
            return super().get(key, None)
        return get(self, key)

    def __setitem__(self, key, value):
        if "." not in key:
            return super().__setitem__(key, value)
        set_(self, key, value, newtype=RecursiveDict)

    def __delitem__(self, key):
        if "." not in key:
            return super().pop(key, None)
        return pop(self, key)

    def __contains__(self, key) -> bool:
        if "." not in key:
            return super().__contains__(key)
        return has(self, key)

    def get(self, key, default=None):
        if "." not in key:
            return super().get(key, default)
        return get(self, key) or default

    def pop(self, key, default=None):
        if "." not in key:
            return super().pop(key, default)
        return pop(self, key, default)


def merge_dicts(d1, d2):
    # force RecursiveDict instances to be treated as regular dicts
    d1_type = dict if isinstance(d1, RecursiveDict) else type(d1)
    d2_type = dict if isinstance(d2, RecursiveDict) else type(d2)
    if d1 is not None and d1_type != d2_type:
        raise TypeError(f"d1 and d2 are of different types: {type(d1)} vs {type(d2)}")
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

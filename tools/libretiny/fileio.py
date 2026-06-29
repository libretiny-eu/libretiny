# Copyright (c) Kuba Szczodrzyński 2022-06-10.

import json
from json import JSONDecodeError
from os.path import isfile
from typing import Optional, Union


def readjson(file: str) -> Optional[Union[dict, list]]:
    """Read a JSON file into a dict or list."""
    if not isfile(file):
        return None
    with open(file, "r", encoding="utf-8") as f:
        try:
            return json.load(f)
        except JSONDecodeError:
            return None

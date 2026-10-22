#  Copyright (c) Kuba Szczodrzyński 2023-3-18.

import json
from pathlib import Path
from typing import Dict, Union

json_cache: Dict[str, Union[list, dict]] = {}
libretiny_path = Path(__file__).parents[2]


def lvm_load_json(path: str) -> Union[list, dict]:
    if path not in json_cache:
        with libretiny_path.joinpath(path).open("rb") as f:
            json_cache[path] = json.load(f)
    return json_cache[path]


def lvm_path() -> Path:
    return libretiny_path

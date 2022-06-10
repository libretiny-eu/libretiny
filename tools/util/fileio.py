# Copyright (c) Kuba Szczodrzyński 2022-06-10.

import json
from io import BytesIO
from os.path import dirname, getmtime, isfile, join
from typing import Union


def chname(path: str, name: str) -> str:
    """Change the basename of 'path' to 'name'."""
    return join(dirname(path), name)


def chext(path: str, ext: str) -> str:
    """Change the file extension of 'path' to 'ext' (without the dot)."""
    return path.rpartition(".")[0] + "." + ext


def isnewer(what: str, than: str) -> bool:
    """Check if 'what' is newer than 'than'.

    Returns False if 'what' is not a file.

    Returns True if 'than' is not a file.
    """
    if not isfile(what):
        return False
    if not isfile(than):
        return True
    return getmtime(what) > getmtime(than)


def readbin(file: str) -> bytes:
    """Read a binary file into a bytes object."""
    with open(file, "rb") as f:
        data = f.read()
    return data


def writebin(file: str, data: Union[bytes, BytesIO]):
    """Write data into a binary file."""
    with open(file, "wb") as f:
        if isinstance(data, BytesIO):
            f.write(data.getvalue())
        else:
            f.write(data)


# same as load_json
def readjson(file: str) -> Union[dict, list]:
    """Read a JSON file into a dict or list."""
    with open(file, "r", encoding="utf-8") as f:
        return json.load(f)

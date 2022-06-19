# Copyright (c) Kuba Szczodrzyński 2022-06-02.

from os.path import dirname, isdir, join
from typing import List


class Family:
    id: int
    short_name: str
    description: str
    name: str = None
    parent: str = None
    code: str = None
    parent_code: str = None
    url: str = None
    sdk: str = None
    framework: str = None
    mcus: List[str] = []

    def __init__(self, data: dict):
        for key, value in data.items():
            if key == "id":
                self.id = int(value, 16)
            else:
                setattr(self, key, value)

    @property
    def sdk_name(self) -> str:
        return self.sdk.rpartition("/")[2] if self.sdk else None

    @property
    def has_arduino_core(self) -> bool:
        if not self.name:
            return False
        if isdir(join(dirname(__file__), "..", "..", "arduino", self.name)):
            return True
        if not self.parent:
            return False
        if isdir(join(dirname(__file__), "..", "..", "arduino", self.parent)):
            return True
        return False

    def dict(self) -> dict:
        return dict(
            FAMILY=self.short_name,
            FAMILY_ID=self.id,
            FAMILY_NAME=self.name,
            FAMILY_PARENT=self.parent,
            FAMILY_CODE=self.code,
            FAMILY_PARENT_CODE=self.parent_code,
        )

    def __eq__(self, __o: object) -> bool:
        return isinstance(__o, Family) and self.id == __o.id

    def __iter__(self):
        return iter(self.dict().items())

    def __repr__(self) -> str:
        return f"<Family: {self.short_name}(0x{self.id:X}), name={self.name}, parent={self.parent}>"

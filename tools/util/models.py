# Copyright (c) Kuba Szczodrzyński 2022-06-02.

from typing import List


class Family:
    id: int
    short_name: str
    description: str
    name: str = None
    code: str = None
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

    def __eq__(self, __o: object) -> bool:
        return isinstance(__o, Family) and self.id == __o.id

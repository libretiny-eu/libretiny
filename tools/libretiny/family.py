# Copyright (c) Kuba Szczodrzyński 2022-06-02.

from dataclasses import dataclass, field
from typing import List, Optional, Union

from .lvm import lvm_load_json, lvm_path

LT_FAMILIES: List["Family"] = []


@dataclass
class Family:
    name: str
    parent: Union["Family", None]
    code: str
    description: str
    id: Optional[int] = None
    short_name: Optional[str] = None
    package: Optional[str] = None
    mcus: List[str] = field(default_factory=lambda: [])
    children: List["Family"] = field(default_factory=lambda: [])

    # noinspection PyTypeChecker
    def __post_init__(self):
        if self.id:
            self.id = int(self.id, 16)
        self.mcus = set(self.mcus)

    @classmethod
    def get_all(cls) -> List["Family"]:
        global LT_FAMILIES
        if LT_FAMILIES:
            return LT_FAMILIES
        families = lvm_load_json("families.json")
        LT_FAMILIES = [
            cls(name=k, **v) for k, v in families.items() if isinstance(v, dict)
        ]
        # attach parents and children to all families
        for family in LT_FAMILIES:
            if family.parent is None:
                continue
            try:
                parent = next(f for f in LT_FAMILIES if f.name == family.parent)
            except StopIteration:
                raise ValueError(
                    f"Family parent '{family.parent}' of '{family.name}' doesn't exist"
                )
            family.parent = parent
            parent.children.append(family)
        return LT_FAMILIES

    @classmethod
    def get(
        cls,
        any: str = None,
        id: Union[str, int] = None,
        short_name: str = None,
        name: str = None,
        code: str = None,
        description: str = None,
    ) -> "Family":
        if any:
            id = any
            short_name = any
            name = any
            code = any
            description = any
        if id and isinstance(id, str) and id.startswith("0x"):
            id = int(id, 16)
        for family in cls.get_all():
            if id and family.id == id:
                return family
            if short_name and family.short_name == short_name.upper():
                return family
            if name and family.name == name.lower():
                return family
            if code and family.code == code.lower():
                return family
            if description and family.description == description:
                return family
        if any:
            raise ValueError(f"Family not found - {any}")
        items = [hex(id) if id else None, short_name, name, code, description]
        text = ", ".join(filter(None, items))
        raise ValueError(f"Family not found - {text}")

    @property
    def has_arduino_core(self) -> bool:
        if lvm_path().joinpath("cores", self.name, "arduino").is_dir():
            return True
        if self.parent:
            return self.parent.has_arduino_core
        return False

    @property
    def target_package(self) -> Optional[str]:
        return self.package or self.parent and self.parent.target_package

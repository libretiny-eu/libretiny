# Copyright (c) Kuba Szczodrzyński 2022-06-10.

from typing import List, Tuple, Union

from tools.util.intbin import uintmax
from tools.util.obj import SliceLike, slice2int


def bitcat(*vars: Tuple[Union["BitInt", int], SliceLike]) -> int:
    """Concat all 'vars' denoted in a (value, slice) format into a bitstring."""
    out = 0
    for val, sl in vars:
        if not isinstance(val, BitInt):
            val = BitInt(val)
        (start, stop) = slice2int(sl)
        out <<= start - stop + 1
        out |= val[start:stop]
    return out


def bitcatraw(*vars: Tuple[int, int]) -> int:
    """Concat all 'vars' denoted in a (value, bitwidth) format into a bitstring."""
    out = 0
    for val, bits in vars:
        out <<= bits
        out |= val
    return out


class BitInt(int):
    """
    Wrapper for int supporting slice reading and assignment of
    individual bits (counting from LSB to MSB, like '7:0').
    """

    value: int = None

    def __init__(self, value: int) -> None:
        self.value = value

    def __getitem__(self, key):
        if self.value is None:
            self.value = self
        # for best performance, slice2int() type checking was disabled
        if isinstance(key, int):
            return (self.value >> key) % 2
        # (start, stop) = slice2int(key)
        return (self.value >> key.stop) & uintmax(key.start - key.stop + 1)

    def __setitem__(self, key, value):
        if self.value is None:
            self.value = self
        (start, stop) = slice2int(key)

        if value > uintmax(start - stop + 1):
            raise ValueError("value is too big")

        tmp = self.value & ~uintmax(start + 1)
        tmp |= self.value & uintmax(stop)
        tmp |= value << stop
        self.value = tmp

    def rep(self, n: int, sl: Union[SliceLike, List[SliceLike]]) -> int:
        """Construct a bitstring from 'sl' (being a single slice or a list)
        repeated 'n' times."""
        if isinstance(sl, list):
            return self.cat(*(sl * n))
        return self.cat(*([sl] * n))

    def cat(self, *slices: SliceLike) -> int:
        """Construct a bitstring from this BitInt's parts denoted by 'slices'."""
        out = 0
        for sl in slices:
            (start, stop) = slice2int(sl)
            out <<= start - stop + 1
            out |= self[start:stop]
        return out

    def __int__(self) -> int:
        return self.value or self

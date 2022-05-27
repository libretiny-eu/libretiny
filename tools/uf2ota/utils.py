# Copyright (c) Kuba Szczodrzyński 2022-05-27.


def bswap(data: bytes) -> bytes:
    return bytes(reversed(data))


def betoint(data: bytes) -> int:
    return int.from_bytes(data, byteorder="big")


def letoint(data: bytes) -> int:
    return int.from_bytes(data, byteorder="little")


def inttobe32(data: int) -> bytes:
    return data.to_bytes(length=4, byteorder="big")


def inttole32(data: int) -> bytes:
    return data.to_bytes(length=4, byteorder="little")


def inttobe24(data: int) -> bytes:
    return data.to_bytes(length=3, byteorder="big")


def inttole24(data: int) -> bytes:
    return data.to_bytes(length=3, byteorder="little")


def inttobe16(data: int) -> bytes:
    return data.to_bytes(length=2, byteorder="big")


def inttole16(data: int) -> bytes:
    return data.to_bytes(length=2, byteorder="little")


def intto8(data: int) -> bytes:
    return data.to_bytes(length=1, byteorder="big")

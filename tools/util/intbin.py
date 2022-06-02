# Copyright (c) Kuba Szczodrzyński 2022-06-02.


def bswap(data: bytes) -> bytes:
    return bytes(reversed(data))


def betoint(data: bytes) -> int:
    return int.from_bytes(data, byteorder="big")


def letoint(data: bytes) -> int:
    return int.from_bytes(data, byteorder="little")


def betosint(data: bytes) -> int:
    return int.from_bytes(data, byteorder="big", signed=True)


def letosint(data: bytes) -> int:
    return int.from_bytes(data, byteorder="little", signed=True)


def inttole32(data: int) -> bytes:
    return data.to_bytes(length=4, byteorder="little")


def inttole24(data: int) -> bytes:
    return data.to_bytes(length=3, byteorder="little")


def inttole16(data: int) -> bytes:
    return data.to_bytes(length=2, byteorder="little")


def intto8(data: int) -> bytes:
    return data.to_bytes(length=1, byteorder="big")


def sinttole32(data: int) -> bytes:
    return data.to_bytes(length=4, byteorder="little", signed=True)


def sinttole24(data: int) -> bytes:
    return data.to_bytes(length=3, byteorder="little", signed=True)


def sinttole16(data: int) -> bytes:
    return data.to_bytes(length=2, byteorder="little", signed=True)


def sintto8(data: int) -> bytes:
    return data.to_bytes(length=1, byteorder="little", signed=True)


def align_up(x: int, n: int) -> int:
    return int((x - 1) // n + 1) * n


def align_down(x: int, n: int) -> int:
    return int(x // n) * n

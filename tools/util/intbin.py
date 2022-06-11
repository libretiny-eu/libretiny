# Copyright (c) Kuba Szczodrzyński 2022-06-02.

from io import FileIO
from typing import IO, Generator, Union

ByteGenerator = Generator[bytes, None, None]


def bswap(data: bytes) -> bytes:
    """Reverse the byte array (big-endian <-> little-endian)."""
    return bytes(reversed(data))


def betoint(data: bytes) -> int:
    """Convert bytes to big-endian unsigned integer."""
    return int.from_bytes(data, byteorder="big")


def letoint(data: bytes) -> int:
    """Convert bytes to little-endian unsigned integer."""
    return int.from_bytes(data, byteorder="little")


def betosint(data: bytes) -> int:
    """Convert bytes to big-endian signed integer."""
    return int.from_bytes(data, byteorder="big", signed=True)


def letosint(data: bytes) -> int:
    """Convert bytes to little-endian signed integer."""
    return int.from_bytes(data, byteorder="little", signed=True)


def inttobe32(data: int) -> bytes:
    """Convert unsigned integer to 32 bits, big-endian."""
    return data.to_bytes(length=4, byteorder="big")


def inttole32(data: int) -> bytes:
    """Convert unsigned integer to 32 bits, little-endian."""
    return data.to_bytes(length=4, byteorder="little")


def inttobe24(data: int) -> bytes:
    """Convert unsigned integer to 24 bits, big-endian."""
    return data.to_bytes(length=3, byteorder="big")


def inttole24(data: int) -> bytes:
    """Convert unsigned integer to 24 bits, little-endian."""
    return data.to_bytes(length=3, byteorder="little")


def inttobe16(data: int) -> bytes:
    """Convert unsigned integer to 16 bits, big-endian."""
    return data.to_bytes(length=2, byteorder="big")


def inttole16(data: int) -> bytes:
    """Convert unsigned integer to 16 bits, little-endian."""
    return data.to_bytes(length=2, byteorder="little")


def intto8(data: int) -> bytes:
    """Convert unsigned integer to 8 bits."""
    return data.to_bytes(length=1, byteorder="big")


def sinttobe32(data: int) -> bytes:
    """Convert signed integer to 32 bits, big-endian."""
    return data.to_bytes(length=4, byteorder="big", signed=True)


def sinttole32(data: int) -> bytes:
    """Convert signed integer to 32 bits, little-endian."""
    return data.to_bytes(length=4, byteorder="little", signed=True)


def sinttobe24(data: int) -> bytes:
    """Convert signed integer to 24 bits, big-endian."""
    return data.to_bytes(length=3, byteorder="big", signed=True)


def sinttole24(data: int) -> bytes:
    """Convert signed integer to 24 bits, little-endian."""
    return data.to_bytes(length=3, byteorder="little", signed=True)


def sinttobe16(data: int) -> bytes:
    """Convert signed integer to 16 bits, big-endian."""
    return data.to_bytes(length=2, byteorder="big", signed=True)


def sinttole16(data: int) -> bytes:
    """Convert signed integer to 16 bits, little-endian."""
    return data.to_bytes(length=2, byteorder="little", signed=True)


def sintto8(data: int) -> bytes:
    """Convert signed integer to 8 bits."""
    return data.to_bytes(length=1, byteorder="little", signed=True)


def align_up(x: int, n: int) -> int:
    """Return x aligned up to block size of n."""
    return int((x - 1) // n + 1) * n


def align_down(x: int, n: int) -> int:
    """Return 'x' aligned down to block size of 'n'."""
    return int(x // n) * n


def pad_up(x: int, n: int) -> int:
    """Return how many bytes of padding is needed to align 'x'
    up to block size of 'n'."""
    return n - (x % n)


def pad_data(data: bytes, n: int, char: int) -> bytes:
    """Add 'char'-filled padding to 'data' to align to a 'n'-sized block."""
    if len(data) % n == 0:
        return data
    return data + (bytes([char]) * pad_up(len(data), n))


def uint8(val):
    """Get only the least-significant 8 bits of the value."""
    return val & 0xFF


def uint16(val):
    """Get only the least-significant 16 bits of the value."""
    return val & 0xFFFF


def uint32(val):
    """Get only the least-significant 32 bits of the value."""
    return val & 0xFFFFFFFF


def uintmax(bits: int) -> int:
    """Get maximum integer size for given bit width."""
    return (2**bits) - 1


def biniter(data: bytes, size: int) -> ByteGenerator:
    """Iterate over 'data' in 'size'-bytes long chunks, returning
    a generator."""
    if len(data) % size != 0:
        raise ValueError(
            f"Data length must be a multiple of block size ({len(data)} % {size})"
        )
    for i in range(0, len(data), size):
        yield data[i : i + size]


def geniter(gen: Union[ByteGenerator, bytes, IO], size: int) -> ByteGenerator:
    """
    Take data from 'gen' and generate 'size'-bytes long chunks.

    If 'gen' is a bytes or IO object, it is wrapped using
    biniter() or fileiter().
    """
    if isinstance(gen, bytes):
        yield from biniter(gen, size)
        return
    if isinstance(gen, IO):
        yield from fileiter(gen, size)
        return
    buf = b""
    for part in gen:
        if not buf and len(part) == size:
            yield part
            continue
        buf += part
        while len(buf) >= size:
            yield buf[0:size]
            buf = buf[size:]


def fileiter(
    f: FileIO, size: int, padding: int = 0x00, count: int = 0
) -> ByteGenerator:
    """
    Read data from 'f' and generate 'size'-bytes long chunks.

    Pad incomplete chunks with 'padding' character.

    Read up to 'count' bytes from 'f', if specified. Data is padded
    if not on chunk boundary.
    """
    read = 0
    while True:
        if count and read + size >= count:
            yield pad_data(f.read(count % size), size, padding)
            return
        data = f.read(size)
        read += len(data)
        if len(data) < size:
            # got only part of the block
            yield pad_data(data, size, padding)
            return
        yield data

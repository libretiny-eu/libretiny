"""Native image2 boundary validation shared by AMB1 host tools."""

import struct


def validate_image2(payload):
    """Check exact native boundaries; no checksum/footer/padding is admitted."""
    if len(payload) < 42:
        raise ValueError("Truncated native image2")
    ram_size, ram_base = struct.unpack_from("<II", payload)
    if ram_base != 0x10006000 or not 10 <= ram_size <= 0x6A000:
        raise ValueError("Invalid SRAM extent")
    if payload[8:16] != b"81958711":
        raise ValueError("Candidate must have the new native boot marker")
    entry = struct.unpack_from("<I", payload, 16)[0]
    if not entry & 1 or not ram_base <= (entry & ~1) < ram_base + ram_size:
        raise ValueError("Invalid Thumb entry")
    if payload[20:26] != b"RTKWin":
        raise ValueError("Missing RTKWin entry-table marker")
    offset = 16 + ram_size
    if offset + 16 > len(payload):
        raise ValueError("Truncated SDRAM header")
    size, base = struct.unpack_from("<II", payload, offset)
    if base != 0x30000000 or size > 0x200000:
        raise ValueError("Invalid SDRAM extent")
    if payload[offset + 8 : offset + 16] != b"\xff" * 8:
        raise ValueError("Invalid SDRAM reserved marker")
    if offset + 16 + size != len(payload):
        raise ValueError("Native segment boundaries do not equal exact payload length")
    return {
        "sram_bytes": ram_size,
        "sdram_bytes": size,
        "sdram_header_offset": offset,
        "payload_bytes": len(payload),
    }

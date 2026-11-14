#!/usr/bin/env python3
"""Emit a real LibreTiny AMB1 UF2 package from a native image2 payload.

This is the host-side counterpart of the fail-closed OTA bridge contract in
``libretiny/cores/realtek-amb1/base/ota/lt_ota_family_amb1.c``:

  * a 512-byte metadata-only header block (``block_seq`` 0, ``len`` 0, tags),
  * contiguous 256-byte data blocks (``block_seq`` 1..N, image-relative
    ``addr`` 0, 256, 512, ...),
  * ``block_count == N + 1`` and ``total_size == block_count * 512`` so the
    bridge's ``image_size = (block_count - 1) * 256`` equals the payload length.

``image2_verify()`` additionally requires ``dram == size - second - 16``. A
native image whose length is not a multiple of 256 therefore cannot simply be
trailed with padding: the pad is appended to the SDRAM segment and the SDRAM
length word is increased by the same amount, preserving that invariant.

The image is validated with the audited ``ota_layout.validate_image2`` gate and
the SDRAM bound is taken from ``package_elf``; extraction itself is never
duplicated here.

The pad is transport-only. ``verify_image2.py`` stays untouched and still audits
the unpadded native image2; in ELF-comparison mode it would report the pad as
unaccounted trailing SDRAM bytes, which is expected and recorded in this
emitter's report instead.
"""

import argparse
import json
import struct
from binascii import crc32
from pathlib import Path

import image2_layout as ota_layout
from package_elf import SDRAM_BASE, SDRAM_END
from package_elf import package as package_elf

UF2_BLOCK_SIZE = 512
UF2_DATA_SIZE = 256
UF2_MAGIC_1 = 0x0A324655
UF2_MAGIC_2 = 0x9E5D5157
UF2_MAGIC_3 = 0x0AB16F30

# lt_cpu_get_family() -> F_RTL8710A (libretiny/cores/common/base/lt_types.h),
# the value lt_ota_begin() hands to uf2_ctx_init().
FAMILY_ID = 0x9FFFD543

# UF2 flag bits (uf2tool/models/flags.py, uf2types.h bitfields).
FLAG_NOT_MAIN_FLASH = 0x00000001
FLAG_FILE_CONTAINER = 0x00001000
FLAG_HAS_FAMILY_ID = 0x00002000
FLAG_HAS_MD5 = 0x00004000
FLAG_HAS_TAGS = 0x00008000

# uf2ota tag identifiers (uf2types.h, ltchiptool/uf2tool/models/enums.py).
TAG_VERSION = 0x9FC7BC
TAG_DEVICE = 0x650D9D
TAG_DEVICE_ID = 0xC8A729
TAG_OTA_FORMAT_2 = 0x6C8492
TAG_OTA_PART_LIST = 0x6EC68A
TAG_OTA_PART_INFO = 0xC0EE0C
TAG_BOARD = 0xCA25C8
TAG_FIRMWARE = 0x00DE43
TAG_LT_VERSION = 0x59563D

# OTA_PART_LIST: one byte per scheme pair (device/flasher), nibbles for the two
# slots. 0x01 enables DEVICE_DUAL_1 (byte 0, low nibble) and 0x10 enables
# DEVICE_DUAL_2 (byte 1, high nibble), matching lt_ota_get_uf2_scheme().
DUAL_DEVICE_PART_LIST = b"\x01\x10\x00"


def resolve_family_id():
    """Return the AMB1 UF2 family ID accepted by uf2_check_block()."""
    return FAMILY_ID


def _require(condition, message):
    if not condition:
        raise ValueError(message)


def encode_tag(tag_type, data):
    """Encode one uf2OTA tag: u8 length (incl. header), u24 LE type, payload."""
    _require(0 <= tag_type <= 0xFFFFFF, "Tag type out of range")
    length = 4 + len(data)
    _require(length <= 0xFF, "Tag payload too long")
    blob = bytes((length,)) + tag_type.to_bytes(3, "little") + data
    return blob + b"\x00" * ((-length) % 4)


def encode_tags(tags):
    """Encode an ordered sequence of ``(tag_type, bytes)`` plus a terminator."""
    blob = b"".join(encode_tag(tag_type, data) for tag_type, data in tags)
    blob += b"\x00\x00\x00\x00"  # zero-length tag ends the chain
    _require(len(blob) <= 476, "Tags do not fit in one UF2 block")
    return blob


def decode_tags(data):
    """Decode a UF2 block's tag area into an ordered ``(type, bytes)`` list."""
    tags = []
    pos = 0
    while pos < len(data):
        length = data[pos]
        if not length:
            break
        _require(length >= 4 and pos + length <= len(data), "Malformed UF2 tag")
        tag_type = data[pos + 1] | (data[pos + 2] << 8) | (data[pos + 3] << 16)
        tags.append((tag_type, data[pos + 4 : pos + length]))
        pos += ((length - 1) // 4 + 1) * 4
    return tags


def build_block(flags, addr, seq, block_count, family_id, data=b"", tags=b""):
    """Build one 512-byte UF2 block. ``tags`` is pre-encoded wire data."""
    _require(len(data) + len(tags) <= 476, "Block payload exceeds 476 bytes")
    header = struct.pack(
        "<IIIIIIII",
        UF2_MAGIC_1,
        UF2_MAGIC_2,
        flags,
        addr,
        len(data),
        seq,
        block_count,
        family_id,
    )
    body = data + tags
    return header + body + b"\x00" * (476 - len(body)) + struct.pack("<I", UF2_MAGIC_3)


def pad_image2(image):
    """Pad image2 to a 256-byte multiple by extending the SDRAM segment.

    Returns ``(padded, report)``. The SDRAM length word is increased by the pad
    so ``image2_verify``'s ``dram == size - second - 16`` still holds and
    ``ota_layout.validate_image2`` still accepts exact boundaries.
    """
    details = ota_layout.validate_image2(image)
    padding = (-len(image)) % UF2_DATA_SIZE
    sdram_before = details["sdram_bytes"]
    sdram_after = sdram_before + padding
    _require(sdram_after <= SDRAM_END - SDRAM_BASE, "Padding exceeds the SDRAM region")
    padded = bytearray(image)
    padded.extend(b"\x00" * padding)
    if padding:
        header_offset = details["sdram_header_offset"]
        struct.pack_into("<I", padded, header_offset, sdram_after)
    padded = bytes(padded)
    # Re-run the audited gate: boundaries must remain exact after the edit.
    ota_layout.validate_image2(padded)
    _require(len(padded) % UF2_DATA_SIZE == 0, "Padded image is not block aligned")
    return padded, {
        "padding_bytes": padding,
        "sram_bytes": details["sram_bytes"],
        "sdram_bytes_before": sdram_before,
        "sdram_bytes_after": sdram_after,
        "padded_bytes": len(padded),
    }


def build_header(tags, block_count):
    """Build the metadata-only header block (seq 0, len 0, not main flash)."""
    flags = FLAG_NOT_MAIN_FLASH | FLAG_HAS_FAMILY_ID | FLAG_HAS_TAGS
    return build_block(flags, 0, 0, block_count, FAMILY_ID, b"", encode_tags(tags))


def build_part_info(partitions):
    """Encode OTA_PART_INFO for a two-slot device package."""
    _require(len(partitions) == 2, "Exactly two OTA partitions are required")
    named = b"\x00".join(name.encode("utf-8") for name in partitions) + b"\x00"
    return b"\x01\x20\x00" + named  # scheme 1 -> part 1, scheme 2 -> part 2


def pack(image, *, fw_name, fw_version, lt_version, board, partitions=("ota1", "ota2")):
    """Pack a native image2 into UF2 bytes plus a JSON-safe report."""
    for name, value in (
        ("fw_name", fw_name),
        ("fw_version", fw_version),
        ("lt_version", lt_version),
        ("board", board),
    ):
        _require(isinstance(value, str) and value, f"Non-empty {name} required")
    _require(board == board.lower(), "Board code must be lowercase")

    padded, pad = pad_image2(image)
    data_blocks = len(padded) // UF2_DATA_SIZE
    block_count = data_blocks + 1

    header_tags = [
        (TAG_BOARD, board.encode("utf-8")),
        (TAG_DEVICE_ID, struct.pack("<I", crc32(("LibreTiny " + board).encode()))),
        (TAG_FIRMWARE, fw_name.encode("utf-8")),
        (TAG_VERSION, fw_version.encode("utf-8")),
        (TAG_LT_VERSION, lt_version.encode("utf-8")),
        (TAG_DEVICE, b"LibreTiny"),
        (TAG_OTA_FORMAT_2, b"\x02"),
        (TAG_OTA_PART_LIST, DUAL_DEVICE_PART_LIST),
    ]
    blocks = [build_header(header_tags, block_count)]
    part_info = encode_tags([(TAG_OTA_PART_INFO, build_part_info(partitions))])

    for index in range(data_blocks):
        chunk = padded[index * UF2_DATA_SIZE : (index + 1) * UF2_DATA_SIZE]
        data_tags = part_info if index == 0 else b""
        flags = FLAG_HAS_FAMILY_ID | (FLAG_HAS_TAGS if data_tags else 0)
        blocks.append(
            build_block(
                flags,
                index * UF2_DATA_SIZE,
                index + 1,
                block_count,
                FAMILY_ID,
                chunk,
                data_tags,
            )
        )

    uf2 = b"".join(blocks)
    _require(len(uf2) == block_count * UF2_BLOCK_SIZE, "UF2 size/block-count mismatch")
    _require(len(uf2) % UF2_BLOCK_SIZE == 0, "UF2 total is not 512-byte aligned")
    report = dict(
        family_id=FAMILY_ID,
        family_id_hex=hex(FAMILY_ID),
        image_bytes=len(image),
        block_count=block_count,
        data_blocks=data_blocks,
        image_size=(block_count - 1) * UF2_DATA_SIZE,
        total_bytes=len(uf2),
        total_blocks_in_file=len(uf2) // UF2_BLOCK_SIZE,
        header_tags=[tag_type for tag_type, _ in header_tags],
        partitions=list(partitions),
        **pad,
    )
    _require(report["image_size"] == len(padded), "image_size must equal padded length")
    return uf2, report


def _block_fields(block):
    _require(len(block) == UF2_BLOCK_SIZE, "Invalid UF2 block size")
    m1, m2, flags, addr, length, seq, count, family = struct.unpack_from(
        "<IIIIIIII", block
    )
    _require(m1 == UF2_MAGIC_1 and m2 == UF2_MAGIC_2, "Invalid UF2 magic 1/2")
    _require(
        struct.unpack_from("<I", block, 508)[0] == UF2_MAGIC_3, "Invalid UF2 magic 3"
    )
    return dict(
        flags=flags, addr=addr, len=length, seq=seq, count=count, family_id=family
    )


def unpack(uf2):
    """Validate and decode a UF2 produced by :func:`pack`.

    Returns the concatenated (padded) image, the decoded header metadata and the
    per-block layout. Every bridge invariant is asserted, not assumed.
    """
    _require(
        uf2 and len(uf2) % UF2_BLOCK_SIZE == 0, "UF2 total is not 512-byte aligned"
    )
    total_blocks = len(uf2) // UF2_BLOCK_SIZE
    _require(total_blocks >= 2, "UF2 must contain a header and at least one data block")

    header = _block_fields(uf2[:UF2_BLOCK_SIZE])
    _require(header["seq"] == 0, "Header must be block_seq 0")
    _require(header["len"] == 0, "Header must carry no data")
    _require(header["flags"] & FLAG_HAS_TAGS, "Header must have tags")
    _require(header["flags"] & FLAG_NOT_MAIN_FLASH, "Header must set not_main_flash")
    _require(
        not header["flags"] & FLAG_FILE_CONTAINER, "Header must not be a file container"
    )
    _require(header["flags"] & FLAG_HAS_FAMILY_ID, "Header must carry a family ID")
    _require(header["family_id"] == FAMILY_ID, "Header family ID mismatch")
    _require(header["count"] == total_blocks, "Header block_count mismatch")

    raw_tags = decode_tags(uf2[32 : UF2_BLOCK_SIZE - 4])
    tag_map = {tag_type: data for tag_type, data in raw_tags}
    info = dict(
        fw_name=tag_map.get(TAG_FIRMWARE, b"").decode("utf-8", "replace"),
        fw_version=tag_map.get(TAG_VERSION, b"").decode("utf-8", "replace"),
        lt_version=tag_map.get(TAG_LT_VERSION, b"").decode("utf-8", "replace"),
        board=tag_map.get(TAG_BOARD, b"").decode("utf-8", "replace"),
    )
    _require(TAG_OTA_FORMAT_2 in tag_map, "Missing OTA_FORMAT_2 tag")
    _require(tag_map[TAG_OTA_FORMAT_2] == b"\x02", "OTA_FORMAT_2 must be version 2")
    _require(
        tag_map.get(TAG_OTA_PART_LIST) == DUAL_DEVICE_PART_LIST,
        "OTA_PART_LIST must enable both dual-device schemes",
    )

    chunks = []
    block_info = []
    for seq in range(1, total_blocks):
        block = uf2[seq * UF2_BLOCK_SIZE : (seq + 1) * UF2_BLOCK_SIZE]
        fields = _block_fields(block)
        _require(fields["seq"] == seq, "Data block sequence is not contiguous")
        _require(fields["count"] == total_blocks, "Data block_count mismatch")
        _require(
            not fields["flags"] & FLAG_NOT_MAIN_FLASH,
            "Data block must target main flash",
        )
        _require(
            not fields["flags"] & FLAG_FILE_CONTAINER,
            "Data block must not be a file container",
        )
        _require(fields["flags"] & FLAG_HAS_FAMILY_ID, "Data block family ID missing")
        _require(fields["family_id"] == FAMILY_ID, "Data block family ID mismatch")
        _require(fields["len"] == UF2_DATA_SIZE, "Data block must be exactly 256 bytes")
        _require(
            fields["addr"] == (seq - 1) * UF2_DATA_SIZE,
            "Data block address must be image-relative and contiguous",
        )
        chunks.append(block[32 : 32 + UF2_DATA_SIZE])
        block_info.append(
            dict(seq=seq, addr=fields["addr"], len=fields["len"], flags=fields["flags"])
        )

    image = b"".join(chunks)
    _require(
        len(image) == (total_blocks - 1) * UF2_DATA_SIZE,
        "Decoded image size does not match block_count",
    )
    details = ota_layout.validate_image2(image)
    return dict(
        family_id=FAMILY_ID,
        block_count=total_blocks,
        image=image,
        image_size=len(image),
        header_flags=header["flags"],
        info=info,
        tags=[tag_type for tag_type, _ in raw_tags],
        blocks=block_info,
        image2=details,
    )


def main(argv=None):
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("--image2", type=Path, help="Native image2 payload")
    parser.add_argument(
        "--elf", type=Path, help="ELF to extract via package_elf.package()"
    )
    parser.add_argument(
        "--toolchain-bin", type=Path, help="arm-none-eabi-* directory (with --elf)"
    )
    parser.add_argument("--output", type=Path, help="UF2 output path (must not exist)")
    parser.add_argument(
        "--board", help="Lowercase board code, e.g. generic-rtl8711am-8mb"
    )
    parser.add_argument("--fw-name", help="Firmware name (FIRMWARE tag)")
    parser.add_argument("--fw-version", help="Firmware version (VERSION tag)")
    parser.add_argument("--lt-version", help="LibreTiny version (LT_VERSION tag)")
    parser.add_argument(
        "--allow-libretiny-image1-overlays",
        action="store_true",
        help="Accept the linker's .valid/.dummy image1 overlay sections",
    )
    parser.add_argument(
        "--inspect", type=Path, help="Validate and describe an existing UF2"
    )
    args = parser.parse_args(argv)

    if args.inspect:
        try:
            result = unpack(args.inspect.read_bytes())
        except (ValueError, OSError) as error:
            parser.error(str(error))
        result.pop("image")
        print(json.dumps(result, indent=2, sort_keys=True))
        return 0

    if (args.image2 is None) == (args.elf is None):
        parser.error("Supply exactly one of --image2 or --elf")
    if args.elf is not None and args.toolchain_bin is None:
        parser.error("--elf requires --toolchain-bin")
    if args.output is None:
        parser.error("--output is required")
    missing = [
        name
        for name in ("board", "fw_name", "fw_version", "lt_version")
        if getattr(args, name) is None
    ]
    if missing:
        parser.error(
            "Missing required metadata: "
            + ", ".join("--" + m.replace("_", "-") for m in missing)
        )
    if args.output.exists():
        parser.error("Output already exists; choose a new output name")

    try:
        if args.elf is not None:
            image, _ = package_elf(
                args.elf, args.toolchain_bin, args.allow_libretiny_image1_overlays
            )
        else:
            image = args.image2.read_bytes()
        uf2, report = pack(
            image,
            fw_name=args.fw_name,
            fw_version=args.fw_version,
            lt_version=args.lt_version,
            board=args.board,
        )
    except (ValueError, OSError) as error:
        parser.error(str(error))

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(uf2)
    report["output"] = str(args.output)
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

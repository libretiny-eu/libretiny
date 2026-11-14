#!/usr/bin/env python3
"""Extract an offline AMB1 native image2; never compose or write SPI flash."""

import argparse
import hashlib
import json
import re
import struct
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path

RAM_BASE, RAM_END = 0x10006000, 0x10070000
SDRAM_BASE, SDRAM_END = 0x30000000, 0x30200000
MAX_IMAGE = 1024 * 1024


@dataclass(frozen=True)
class Section:
    name: str
    size: int
    vma: int
    lma: int
    offset: int
    flags: frozenset


def parse_sections(text):
    sections = []
    lines = text.splitlines()
    pattern = r"^\s*\d+\s+(\S+)\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\s+2\*\*\d+\s*$"
    for index, line in enumerate(lines):
        match = re.match(pattern, line)
        if match:
            if index + 1 == len(lines):
                raise ValueError("Missing objdump section flags")
            name, *numbers = match.groups()
            sections.append(
                Section(
                    name,
                    *(int(n, 16) for n in numbers),
                    frozenset(lines[index + 1].strip().split(", ")),
                )
            )
    if not sections or len({s.name for s in sections}) != len(sections):
        raise ValueError("Missing or duplicate section names")
    return sections


def parse_symbols(text):
    result = {}
    for line in text.splitlines():
        fields = line.split()
        if len(fields) == 3 and re.fullmatch("[0-9a-fA-F]+", fields[0]):
            result[fields[2]] = int(fields[0], 16)
    return result


def digest(data):
    return hashlib.sha256(data).hexdigest()


def build_image(sections, payloads, symbols=None, allow_image1_overlays=False):
    """Package every initialized ALLOC section, with one explicit legacy exception."""
    symbols = symbols or {}
    groups = {"sram": [], "sdram": []}
    excluded = []
    for section in sections:
        if not section.size or not {"ALLOC", "CONTENTS"} <= section.flags:
            continue
        data = payloads[section.name]
        if len(data) != section.size:
            raise ValueError("Section size mismatch: " + section.name)
        # LibreTiny's OVERLAY carries image1 code and zero TCM workspace in EXEC ELFs.
        # These are not image2. Never silently discard arbitrary out-of-range data.
        if allow_image1_overlays and section.name in (".valid", ".dummy"):
            valid = (
                section.vma == 0x1FFF0000
                and section.size <= 0x10000
                and symbols.get("__ram_image1_text_start__") == 0x1FFF0000
                and symbols.get("__tcm_bss_start__") == 0x1FFF0000
            )
            if section.name == ".valid":
                valid = (
                    valid
                    and section.lma == section.vma
                    and not any(data)
                    and symbols.get("__tcm_bss_end__") == section.vma + section.size
                )
            else:
                valid = (
                    valid
                    and "CODE" in section.flags
                    and section.lma == symbols.get("__tcm_bss_end__")
                    and section.vma
                    <= symbols.get("__ram_image1_data_end__", -1)
                    <= section.vma + section.size
                )
            if not valid:
                raise ValueError("Unrecognized image1 overlay: " + section.name)
            excluded.append(
                dict(
                    name=section.name,
                    size=section.size,
                    sha256=digest(data),
                    reason="Explicitly excluded LibreTiny image1/TCM overlay",
                )
            )
            continue
        group = next(
            (
                name
                for name, begin, end in (
                    ("sram", RAM_BASE, RAM_END),
                    ("sdram", SDRAM_BASE, SDRAM_END),
                )
                if begin <= section.vma < section.vma + section.size <= end
            ),
            None,
        )
        if group is None or section.lma != section.vma or "LOAD" not in section.flags:
            raise ValueError("Unsupported initialized ALLOC section: " + section.name)
        groups[group].append(section)
    blobs, manifest = [], []
    for group, base in (("sram", RAM_BASE), ("sdram", SDRAM_BASE)):
        selected = sorted(groups[group], key=lambda s: s.vma)
        if not selected or selected[0].vma != base:
            raise ValueError("Missing segment base: " + group)
        end = max(s.vma + s.size for s in selected)
        payload = bytearray(end - base)
        cursor = base
        for section in selected:
            if section.vma < cursor:
                raise ValueError("Overlapping initialized sections: " + section.name)
            payload[section.vma - base : section.vma - base + section.size] = payloads[
                section.name
            ]
            cursor = section.vma + section.size
            manifest.append(
                dict(
                    name=section.name,
                    segment=group,
                    vma=hex(section.vma),
                    lma=hex(section.lma),
                    size=section.size,
                    flags=sorted(section.flags),
                    sha256=digest(payloads[section.name]),
                )
            )
        blobs.append(bytes(payload))
    ram, sdram = blobs
    table = next((s for s in groups["sram"] if s.name == ".image2.start.table"), None)
    if table is None or table.vma != RAM_BASE or table.size < 10:
        raise ValueError("Missing image2 entry table at SRAM base")
    entry = struct.unpack_from("<I", ram)[0]
    target = entry & ~1
    if not entry & 1 or not any(
        "CODE" in s.flags and s.vma <= target and target + 2 <= s.vma + s.size
        for s in groups["sram"]
    ):
        raise ValueError("Entry must be Thumb in a packaged executable SRAM section")
    if ram[4:10] != b"RTKWin":
        raise ValueError("Missing RTKWin marker")
    arrays = []
    for kind in ("preinit", "init", "fini"):
        start, end = symbols.get("__" + kind + "_array_start"), symbols.get(
            "__" + kind + "_array_end"
        )
        if start is None and end is None:
            continue
        if start is None or end is None or start % 4 or end % 4 or end < start:
            raise ValueError("Invalid constructor array bounds: " + kind)
        if start != end and not any(
            s.vma <= start <= end <= s.vma + s.size
            for s in groups["sram"] + groups["sdram"]
        ):
            raise ValueError("Constructor array is not packaged: " + kind)
        arrays.append(dict(kind=kind, start=hex(start), end=hex(end), size=end - start))
    image = (
        struct.pack("<II", len(ram), RAM_BASE)
        + b"81958711"
        + ram
        + struct.pack("<II", len(sdram), SDRAM_BASE)
        + b"\xff" * 8
        + sdram
    )
    if len(image) > MAX_IMAGE:
        raise ValueError("Application exceeds reviewed 1 MiB limit")
    return image, dict(
        entry_table_pointer=hex(entry),
        sections=manifest,
        excluded_initialized_sections=excluded,
        arrays=arrays,
        segments=[
            dict(base=hex(base), size=len(blob), sha256=digest(blob))
            for base, blob in zip((RAM_BASE, SDRAM_BASE), blobs)
        ],
    )


def package(elf, toolchain_bin, allow_image1_overlays=False):
    elf, toolchain_bin = Path(elf).resolve(), Path(toolchain_bin).resolve()
    raw = elf.read_bytes()
    if (
        len(raw) < 52
        or raw[:7] != b"\x7fELF\x01\x01\x01"
        or struct.unpack_from("<HH", raw, 16) != (2, 40)
    ):
        raise ValueError("Expected a little-endian ELF32 ARM executable")

    def run(tool, *args):
        return subprocess.check_output(
            [str(toolchain_bin / ("arm-none-eabi-" + tool)), *map(str, args)], text=True
        )

    header = run("readelf", "-h", elf)
    sections = parse_sections(run("objdump", "-h", elf))
    symbols = parse_symbols(run("nm", "-n", elf))
    payloads = {}
    with tempfile.TemporaryDirectory(prefix="amb1-elf-") as directory:
        for index, section in enumerate(sections):
            if section.size and {"ALLOC", "CONTENTS"} <= section.flags:
                output = Path(directory) / str(index)
                run("objcopy", "-j", section.name, "-O", "binary", elf, output)
                data = output.read_bytes()
                if data != raw[section.offset : section.offset + section.size]:
                    raise ValueError(
                        "ELF/objcopy section bytes disagree: " + section.name
                    )
                payloads[section.name] = data
    image, report = build_image(sections, payloads, symbols, allow_image1_overlays)
    report["section_inventory"] = [
        dict(
            name=s.name,
            size=s.size,
            vma=hex(s.vma),
            lma=hex(s.lma),
            file_offset=hex(s.offset),
            flags=sorted(s.flags),
            initialized_alloc=bool(s.size and {"ALLOC", "CONTENTS"} <= s.flags),
        )
        for s in sections
    ]
    report.update(
        elf=str(elf),
        elf_size=len(raw),
        elf_sha256=digest(raw),
        elf_entry_ignored=hex(struct.unpack_from("<I", raw, 24)[0]),
        elf_header=header,
        toolchain_bin=str(toolchain_bin),
        application_size=len(image),
        application_sha256=digest(image),
        hardware_tested=False,
        flash_ready=False,
        full_flash_image_created=False,
        limitations=[
            "Offline image2 only; no SPI writes, OTA signing, or credentials",
            "Startup, SDRAM initialization, and hardware operation unverified",
        ],
    )
    return image, report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--elf", required=True, type=Path)
    parser.add_argument("--toolchain-bin", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument(
        "--allow-libretiny-image1-overlays",
        action="store_true",
        help="Explicitly exclude validated .valid/.dummy TCM overlays; reported in manifest",
    )
    args = parser.parse_args()
    manifest = args.output.with_suffix(".json")
    if args.output == manifest or args.output.exists() or manifest.exists():
        parser.error("Output or manifest already exists; choose a new output name")
    try:
        image, report = package(
            args.elf, args.toolchain_bin, args.allow_libretiny_image1_overlays
        )
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        parser.error(str(error))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("xb") as stream:
        stream.write(image)
    with manifest.open("x") as stream:
        json.dump(report, stream, indent=2)
        stream.write("\n")
    print(
        f"Offline image2: {args.output} ({len(image)} bytes); NOT authorized for flashing"
    )


if __name__ == "__main__":
    main()

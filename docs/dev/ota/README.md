# UF2-based OTA

LibreTiny's OTA updating is based on [Microsoft's UF2 specification](https://microsoft.github.io/uf2/). Some aspects of the process, such as OTA1/2 support and target partition selection, have been customized with extension tags.

!!! note
	Just like in UF2, all values in this format are little-endian.

## Firmware images

UF2 files may contain multiple firmware images that are to be flashed, i.e. main firmware + bootloader + some config partition.

Some CPUs support dual-OTA schemes: firmware runs from one image, while the other one is reserved for updated firmware. After applying the update, a reboot causes to run the other image instead.

Each firmware image may be either applicable:

1. only when flashing OTA1 (`part;file;;`)
2. only when flashing OTA2 (`;;part;file`)
3. for both schemes to a single partition (`part;file`)
4. for both schemes but different partitions (`part1;file;part2;file`)
5. for both schemes but with a different binary (`part;file1;part;file2`)
6. for both schemes, with different binaries and target partitions (`part1;file1;part2;file2`)

\* *`part` means partition here*

\*\* values in parentheses show the input format to use for [`uf2ota.py`](uf2ota.md)

For easier understanding, these update types will be referred to in this document using the numbers.

## Extension tags

Standard tags are used: `VERSION`, `DEVICE` and `DEVICE_ID`.

Additionally, custom tags are defined:

Name          | ID       | Type       | Description
--------------|----------|------------|-------------------------------------------------
`OTA_VERSION` | 0x5D57D0 | int 8-bit  | format version (for simple compatibility checks)
`BOARD`       | 0xCA25C8 | string     | board name / code (lowercase)
`FIRMWARE`    | 0x00DE43 | string     | firmware description / name
`BUILD_DATE`  | 0x822F30 | int 32-bit | build date/time as Unix timestamp
`LT_VERSION`  | 0x59563D | semver     | LT version
`LT_PART_1`   | 0x805946 | string     | OTA1 partition name
`LT_PART_2`   | 0xA1E4D7 | string     | OTA2 partition name
`LT_HAS_OTA1` | 0xBBD965 | bool 8-bit | image has any data for OTA1
`LT_HAS_OTA2` | 0x92280E | bool 8-bit | image has any data for OTA2
`LT_BINPATCH` | 0xB948DE | bytes      | binary patch to convert OTA1->OTA2

## Update types

### Single OTA scheme (1, 2)

Image is ignored if the OTA scheme does not match. UF2 has `LT_PART_1` or `LT_PART_2` set to target partition name. The other partition tag is present, but empty (0 bytes).

```
08 46 59 80 6f 74 61 31 | .FY.ota1 | LT_PART_1
04 d7 e4 a1             | ....     | LT_PART_2
```

### Dual-OTA/single-file scheme (3, 4)

One image is used for both OTA schemes. UF2 has `LT_PART_1` and `LT_PART_2` tags set. For type `3` these two tags contain the same partition name.

```
08 46 59 80 6f 74 61 31 | .FY.ota1 | LT_PART_1
08 d7 e4 a1 6f 74 61 32 | ....ota2 | LT_PART_2
```

### Dual-OTA/dual-file scheme (5, 6)

Just like types `3` and `4`, UF2 has two partition tags set. For type `5` they have the same name.

The image stored in UF2 is meant for OTA1 scheme. There is an additional tag `LT_BINPATCH` present. In OTA1 flashing scheme, it is ignored.

## Binary patching

OTA2 images are not stored directly, as that would needlessly double the UF2 file size. Instead, binary patching instructions, embedded into the extension tags area, allow the CPU to convert the OTA1 image from UF2 into OTA2 image.

There can be at most one binpatch tag in a UF2 block. It has the following format:

- opcode (1 byte) - operation type:
	- `DIFF32` (0xFE) - difference between 32-bit values
- length (1 byte) - data length
- data (`length` bytes)
	- for `DIFF32`:
		- difference value (signed int 32-bit)
		- offset table (`length-4` bytes)

The presented structure can be repeated in a single binpatch tag.

### DIFF32

This method works by adding the difference value to a 32-bit integer. It allows to save the most space in OTA1/2 image scenarios, where the only different values are, for example, flash memory addresses. The offset table contains positions within the 256-byte block, to which the difference value should be mathematically added.

For a block like:
```
000  72 71 73 61 76 65 00 00  5f 66 72 65 65 72 74 6f  |rqsave.._freerto|
010  73 5f 6d 75 74 65 78 5f  67 65 74 5f 74 69 6d 65  |s_mutex_get_time|
020  6f 75 74 00 5d a4 03 08  61 a4 03 08 85 a4 03 08  |out.]...a.......|
030  5d a4 03 08 61 a4 03 08  85 a4 03 08 81 a9 03 08  |]...a...........|
040  6d a9 03 08 7d a4 03 08  d9 a8 03 08 05 a7 03 08  |m...}...........|
050  bd a4 03 08 ad a8 03 08  59 a7 03 08 9d a8 03 08  |........Y.......|
060  01 a7 03 08 51 a8 03 08  21 aa 03 08 b9 a4 03 08  |....Q...!.......|
070  85 a3 03 08 89 a3 03 08  4d a4 03 08 a1 a8 03 08  |........M.......|
080  00 00 00 00 00 00 00 00  19 a8 03 08 c1 a4 03 08  |................|
090  8d a8 03 08 ed a6 03 08  dd a7 03 08 ad a4 03 08  |................|
0a0  9d a7 03 08 95 a4 03 08  81 a7 03 08 09 a7 03 08  |................|
0b0  31 a7 03 08 d1 a6 03 08  dd a5 03 08 61 aa 03 08  |1...........a...|
0c0  c5 a2 03 08 d5 a2 03 08  d9 a2 03 08 b1 a6 03 08  |................|
0d0  65 aa 03 08 ad a6 03 08  a9 a6 03 08 8d a6 03 08  |e...............|
0e0  e5 a2 03 08 e9 a2 03 08  1d a4 03 08 ed a3 03 08  |................|
0f0  35 a4 03 08 05 a4 03 08  bd a3 03 08 8d a3 03 08  |5...............|
```

a DIFF32 patch containing:
```
     fe 39 00 50 0c 00 24 28  2c 30 34 38 3c 40 44 48  |.9.P..$(,048<@DH|
     4c 50 54 58 5c 60 64 68  6c 70 74 78 7c 88 8c 90  |LPTX\`dhlptx|...|
     94 98 9c a0 a4 a8 ac b0  b4 b8 bc c0 c4 c8 cc d0  |................|
     d4 d8 dc e0 e4 e8 ec f0  f4 f8 fc                 |...........     |
```

adds 0x000C5000 to 53 values, producing OTA2 output like this:
```
000  72 71 73 61 76 65 00 00  5f 66 72 65 65 72 74 6f  |rqsave.._freerto|
010  73 5f 6d 75 74 65 78 5f  67 65 74 5f 74 69 6d 65  |s_mutex_get_time|
020  6f 75 74 00 5d f4 0f 08  61 f4 0f 08 85 f4 0f 08  |out.]...a.......|
030  5d f4 0f 08 61 f4 0f 08  85 f4 0f 08 81 f9 0f 08  |]...a...........|
040  6d f9 0f 08 7d f4 0f 08  d9 f8 0f 08 05 f7 0f 08  |m...}...........|
050  bd f4 0f 08 ad f8 0f 08  59 f7 0f 08 9d f8 0f 08  |........Y.......|
060  01 f7 0f 08 51 f8 0f 08  21 fa 0f 08 b9 f4 0f 08  |....Q...!.......|
070  85 f3 0f 08 89 f3 0f 08  4d f4 0f 08 a1 f8 0f 08  |........M.......|
080  00 00 00 00 00 00 00 00  19 f8 0f 08 c1 f4 0f 08  |................|
090  8d f8 0f 08 ed f6 0f 08  dd f7 0f 08 ad f4 0f 08  |................|
0a0  9d f7 0f 08 95 f4 0f 08  81 f7 0f 08 09 f7 0f 08  |................|
0b0  31 f7 0f 08 d1 f6 0f 08  dd f5 0f 08 61 fa 0f 08  |1...........a...|
0c0  c5 f2 0f 08 d5 f2 0f 08  d9 f2 0f 08 b1 f6 0f 08  |................|
0d0  65 fa 0f 08 ad f6 0f 08  a9 f6 0f 08 8d f6 0f 08  |e...............|
0e0  e5 f2 0f 08 e9 f2 0f 08  1d f4 0f 08 ed f3 0f 08  |................|
0f0  35 f4 0f 08 05 f4 0f 08  bd f3 0f 08 8d f3 0f 08  |5...............|
```

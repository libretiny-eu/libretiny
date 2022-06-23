# Copyright (c) Kuba Szczodrzyński 2022-05-27.

from enum import IntEnum


class Tag(IntEnum):
    VERSION = 0x9FC7BC  # version of firmware file - UTF8 semver string
    PAGE_SIZE = 0x0BE9F7  # page size of target device (32 bit unsigned number)
    SHA2 = 0xB46DB0  # SHA-2 checksum of firmware (can be of various size)
    DEVICE = 0x650D9D  # description of device (UTF8)
    DEVICE_ID = 0xC8A729  # device type identifier
    # LibreTuya custom tags
    OTA_VERSION = 0x5D57D0  # format version
    BOARD = 0xCA25C8  # board name (lowercase code)
    FIRMWARE = 0x00DE43  # firmware description / name
    BUILD_DATE = 0x822F30  # build date/time as Unix timestamp
    LT_VERSION = 0x59563D  # LT version (semver)
    LT_PART_1 = 0x805946  # OTA1 partition name
    LT_PART_2 = 0xA1E4D7  # OTA2 partition name
    LT_HAS_OTA1 = 0xBBD965  # image has any data for OTA1
    LT_HAS_OTA2 = 0x92280E  # image has any data for OTA2
    LT_BINPATCH = 0xB948DE  # binary patch to convert OTA1->OTA2


class Opcode(IntEnum):
    DIFF32 = 0xFE  # difference between 32-bit values


class Flags:
    not_main_flash: bool = False
    file_container: bool = False
    has_family_id: bool = False
    has_md5: bool = False
    has_tags: bool = False

    def encode(self) -> int:
        val = 0
        if self.not_main_flash:
            val |= 0x00000001
        if self.file_container:
            val |= 0x00001000
        if self.has_family_id:
            val |= 0x00002000
        if self.has_md5:
            val |= 0x00004000
        if self.has_tags:
            val |= 0x00008000
        return val

    def decode(self, data: int):
        self.not_main_flash = (data & 0x00000001) != 0
        self.file_container = (data & 0x00001000) != 0
        self.has_family_id = (data & 0x00002000) != 0
        self.has_md5 = (data & 0x00004000) != 0
        self.has_tags = (data & 0x00008000) != 0

    def __str__(self) -> str:
        flags = []
        if self.not_main_flash:
            flags.append("NMF")
        if self.file_container:
            flags.append("FC")
        if self.has_family_id:
            flags.append("FID")
        if self.has_md5:
            flags.append("MD5")
        if self.has_tags:
            flags.append("TAG")
        return ",".join(flags)


class Input:
    ota1_part: str = None
    ota1_offs: int = 0
    ota1_file: str = None
    ota2_part: str = None
    ota2_offs: int = 0
    ota2_file: str = None

    def __init__(self, input: str) -> None:
        input = input.split(";")
        n = len(input)
        if n not in [2, 4]:
            raise ValueError(
                "Incorrect input format - should be part+offs;file[;part+offs;file]"
            )
        # just spread the same image twice for single-OTA scheme
        if n == 2:
            input += input

        if input[0] and input[1]:
            if "+" in input[0]:
                (self.ota1_part, self.ota1_offs) = input[0].split("+")
                self.ota1_offs = int(self.ota1_offs, 0)
            else:
                self.ota1_part = input[0]
            self.ota1_file = input[1]
        if input[2] and input[3]:
            if "+" in input[2]:
                (self.ota2_part, self.ota2_offs) = input[2].split("+")
                self.ota2_offs = int(self.ota2_offs, 0)
            else:
                self.ota2_part = input[2]
            self.ota2_file = input[3]

        if self.ota1_file and self.ota2_file and self.ota1_offs != self.ota2_offs:
            # currently, offsets cannot differ when storing images
            # (this would require to actually store it twice)
            raise ValueError(f"Offsets cannot differ ({self.ota1_file})")

    @property
    def is_single(self) -> bool:
        return self.ota1_part == self.ota2_part and self.ota1_file == self.ota2_file

    @property
    def single_part(self) -> str:
        return self.ota1_part or self.ota2_part

    @property
    def single_offs(self) -> int:
        return self.ota1_offs or self.ota2_offs

    @property
    def single_file(self) -> str:
        return self.ota1_file or self.ota2_file

    @property
    def has_ota1(self) -> bool:
        return not not (self.ota1_part and self.ota1_file)

    @property
    def has_ota2(self) -> bool:
        return not not (self.ota2_part and self.ota2_file)

    @property
    def is_simple(self) -> bool:
        return self.ota1_file == self.ota2_file or not (self.has_ota1 and self.has_ota2)

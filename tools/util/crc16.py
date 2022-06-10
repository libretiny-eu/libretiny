# Copyright (c) Kuba Szczodrzyński 2022-06-02.

from enum import Enum
from typing import List


class CRC16(Enum):
    # based on https://crccalc.com/ and https://reveng.sourceforge.io/crc-catalogue/16.htm
    ANSI = dict(poly=0x8005, init=0x0000, ref=False, out=0x0000)
    ARC = dict(poly=0x8005, init=0x0000, ref=True, out=0x0000)
    AUG_CCITT = dict(poly=0x1021, init=0x1D0F, ref=False, out=0x0000)
    AUTOSAR = dict(poly=0x1021, init=0xFFFF, ref=False, out=0x0000)
    BUYPASS = dict(poly=0x8005, init=0x0000, ref=False, out=0x0000)
    CCITT = dict(poly=0x1021, init=0x0000, ref=True, out=0x0000)
    CCITT_FALSE = dict(poly=0x1021, init=0xFFFF, ref=False, out=0x0000)
    CCITT_TRUE = dict(poly=0x1021, init=0x0000, ref=True, out=0x0000)
    CDMA2000 = dict(poly=0xC867, init=0xFFFF, ref=False, out=0x0000)
    CMS = dict(poly=0x8005, init=0xFFFF, ref=False, out=0x0000)
    CRC_A = dict(poly=0x1021, init=0xC6C6, ref=True, out=0x0000)
    CRC_B = dict(poly=0x1021, init=0xFFFF, ref=True, out=0xFFFF)
    DARC = dict(poly=0x1021, init=0xFFFF, ref=False, out=0xFFFF)
    DDS_110 = dict(poly=0x8005, init=0x800D, ref=False, out=0x0000)
    DECT_R = dict(poly=0x0589, init=0x0000, ref=False, out=0x0001)
    DECT_X = dict(poly=0x0589, init=0x0000, ref=False, out=0x0000)
    DNP = dict(poly=0x3D65, init=0x0000, ref=True, out=0xFFFF)
    EN_13757 = dict(poly=0x3D65, init=0x0000, ref=False, out=0xFFFF)
    EPC = dict(poly=0x1021, init=0xFFFF, ref=False, out=0xFFFF)
    EPC_C1G2 = dict(poly=0x1021, init=0xFFFF, ref=False, out=0xFFFF)
    GENIBUS = dict(poly=0x1021, init=0xFFFF, ref=False, out=0xFFFF)
    GSM = dict(poly=0x1021, init=0x0000, ref=False, out=0xFFFF)
    I_CODE = dict(poly=0x1021, init=0xFFFF, ref=False, out=0xFFFF)
    IBM = dict(poly=0x8005, init=0x0000, ref=False, out=0x0000)
    IBM_3740 = dict(poly=0x1021, init=0xFFFF, ref=False, out=0x0000)
    IBM_SDLC = dict(poly=0x1021, init=0xFFFF, ref=True, out=0xFFFF)
    IEC_61158_2 = dict(poly=0x1DCF, init=0xFFFF, ref=False, out=0xFFFF)
    ISO_14443_3_A = dict(poly=0x1021, init=0xC6C6, ref=True, out=0x0000)
    ISO_14443_3_B = dict(poly=0x1021, init=0xFFFF, ref=True, out=0xFFFF)
    ISO_HDLC = dict(poly=0x1021, init=0xFFFF, ref=True, out=0xFFFF)
    KERMIT = dict(poly=0x1021, init=0x0000, ref=True, out=0x0000)
    LHA = dict(poly=0x8005, init=0x0000, ref=True, out=0x0000)
    LJ1200 = dict(poly=0x6F63, init=0x0000, ref=False, out=0x0000)
    M17 = dict(poly=0x5935, init=0xFFFF, ref=False, out=0x0000)
    MAXIM = dict(poly=0x8005, init=0x0000, ref=True, out=0xFFFF)
    MCRF4XX = dict(poly=0x1021, init=0xFFFF, ref=True, out=0x0000)
    MODBUS = dict(poly=0x8005, init=0xFFFF, ref=True, out=0x0000)
    NRSC_5 = dict(poly=0x080B, init=0xFFFF, ref=True, out=0x0000)
    OPENSAFETY_A = dict(poly=0x5935, init=0x0000, ref=False, out=0x0000)
    OPENSAFETY_B = dict(poly=0x755B, init=0x0000, ref=False, out=0x0000)
    PROFIBUS = dict(poly=0x1DCF, init=0xFFFF, ref=False, out=0xFFFF)
    RIELLO = dict(poly=0x1021, init=0xB2AA, ref=True, out=0x0000)
    SPI_FUJITSU = dict(poly=0x1021, init=0x1D0F, ref=False, out=0x0000)
    T10_DIF = dict(poly=0x8BB7, init=0x0000, ref=False, out=0x0000)
    TELEDISK = dict(poly=0xA097, init=0x0000, ref=False, out=0x0000)
    TMS37157 = dict(poly=0x1021, init=0x89EC, ref=True, out=0x0000)
    UMTS = dict(poly=0x8005, init=0x0000, ref=False, out=0x0000)
    USB = dict(poly=0x8005, init=0xFFFF, ref=True, out=0xFFFF)
    V_41_LSB = dict(poly=0x1021, init=0x0000, ref=True, out=0x0000)
    VERIFONE = dict(poly=0x8005, init=0x0000, ref=False, out=0x0000)
    X_25 = dict(poly=0x1021, init=0xFFFF, ref=True, out=0xFFFF)
    XMODEM = dict(poly=0x1021, init=0x0000, ref=False, out=0x0000)

    poly: int
    init: int
    ref: bool
    out: int
    table: List[int]

    def __init__(self, params: dict) -> None:
        super().__init__()
        self.poly = params["poly"]
        self.init = params["init"]
        self.ref = params["ref"]
        self.out = params["out"]
        self.table = None
        if self.ref:
            self.poly = self.reverse16(self.poly)
            self.init = self.reverse16(self.init)

    @staticmethod
    def reverse16(num: int) -> int:
        out = 0
        for i in range(16):
            out |= ((num & (1 << i)) >> i) << (15 - i)
        return out

    def calc(self, data: bytes) -> int:
        if self.ref:
            self._init_ref()
            return self._calc_ref(data)
        self._init_std()
        return self._calc_std(data)

    def _init_std(self):
        if self.table:
            return
        self.table = []
        for b in range(256):
            crc = b << 8
            for _ in range(8):
                if crc & 0x8000:
                    crc <<= 1
                    crc ^= self.poly
                else:
                    crc <<= 1
            self.table.append(crc & 0xFFFF)

    def _init_ref(self):
        if self.table:
            return
        self.table = []
        for b in range(256):
            crc = b
            for _ in range(8):
                if crc & 0x0001:
                    crc >>= 1
                    crc ^= self.poly
                else:
                    crc >>= 1
            self.table.append(crc)

    def _calc_std(self, data: bytes) -> int:
        crc = self.init
        for b in data:
            b ^= crc // 256
            crc = self.table[b] ^ (crc * 256 % 0x10000)
        return crc ^ self.out

    def _calc_ref(self, data: bytes) -> int:
        crc = self.init
        for b in data:
            b ^= crc % 256
            crc = self.table[b] ^ (crc // 256)
        return crc ^ self.out

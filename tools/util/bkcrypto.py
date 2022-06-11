# Copyright (c) Kuba Szczodrzyński 2022-06-10.

from typing import List, Tuple

from tools.util.bitint import BitInt, bitcatraw


def pn15(addr: int) -> int:
    # wire [15:0] pn_tmp = {addr[6:0], addr[15:7]} ^ {16'h6371 & {4{addr[8:5]}}};
    a = ((addr % 0x80) * 0x200) + ((addr // 0x80) % 0x200)
    b = (addr // 0x20) % 0x10
    c = 0x6371 & (b * 0x1111)
    return a ^ c


def pn16(addr: int) -> int:
    # wire [16:0] pn_tmp = {addr[9:0], addr[16:10]} ^ {17'h13659 & {addr[4],{4{addr[1],addr[5],addr[9],addr[13]}}}};
    a = ((addr % 0x400) * 0x80) + ((addr // 0x400) % 0x80)
    b = (addr // 0x2000) % 2
    b += ((addr // 0x200) % 2) * 2
    b += ((addr // 0x20) % 2) * 4
    b += ((addr // 0x2) % 2) * 8
    c = (addr // 0x10) % 2
    d = 0x13659 & (c * 0x10000 + b * 0x1111)
    return a ^ d


def pn32(addr: int) -> int:
    # wire [31:0] pn_tmp = {addr[14:0], addr[31:15]} ^ {32'hE519A4F1 & {8{addr[5:2]}}};
    a = ((addr % 0x8000) * 0x20000) + ((addr // 0x8000) % 0x20000)
    b = (addr // 0x4) % 0x10
    c = 0xE519A4F1 & (b * 0x11111111)
    return a ^ c


class BekenCrypto:
    # translated from https://github.com/ghsecuritylab/tysdk_for_bk7231/blob/master/toolchain/encrypt_crc/abc.c
    coef0: BitInt
    coef1_mix: int
    coef1_hi16: int
    bypass: bool = False
    pn15_args: List[slice] = None
    pn16_args: slice = None
    pn32_args: Tuple[int, int] = None
    random: int = 0

    def __init__(self, coeffs: List[BitInt]) -> None:
        (self.coef0, coef1, coef2, coef3) = coeffs

        # wire g_bypass = (coef3[31:24] == 8'hFF) | (coef3[31:24] == 8'h00);
        self.bypass = coef3[31:24] in [0x00, 0xFF]
        if self.bypass:
            return

        # wire       pn16_bit = coef3[4];
        # wire[16:0] pn16_addr = pn16_A ^ {coef1[15:8], pn16_bit, coef1[7:0]};
        self.coef1_mix = bitcatraw((coef1[15:8], 8), (coef3[4], 1), (coef1[7:0], 8))
        self.coef1_hi16 = coef1[31:16]

        # wire pn15_bps = g_bypass | coef3[0];
        pn15_bps = coef3[0]
        # wire pn16_bps = g_bypass | coef3[1];
        pn16_bps = coef3[1]
        # wire pn32_bps = g_bypass | coef3[2];
        pn32_bps = coef3[2]
        # wire rand_bps = g_bypass | coef3[3];
        rand_bps = coef3[3]

        if coef3[3:0] == 0xF:
            self.bypass = True
            return

        if not pn15_bps:
            # wire[1:0] pn15_sel = coef3[ 6: 5];
            pn15_sel = coef3[6:5]
            # wire[15:0] pn15_A = (pn15_sel == 0) ? ({addr[31:24], addr[23:16]} ^ {addr[15:8], addr[ 7:0]}) :
            #                     (pn15_sel == 1) ? ({addr[31:24], addr[23:16]} ^ {addr[ 7:0], addr[15:8]}) :
            #                     (pn15_sel == 2) ? ({addr[23:16], addr[31:24]} ^ {addr[15:8], addr[ 7:0]}) :
            #                                       ({addr[23:16], addr[31:24]} ^ {addr[ 7:0], addr[15:8]});
            if pn15_sel == 0:
                self.pn15_args = [
                    slice(31, 24),
                    slice(23, 16),
                    slice(15, 8),
                    slice(7, 0),
                ]
            elif pn15_sel == 1:
                self.pn15_args = [
                    slice(31, 24),
                    slice(23, 16),
                    slice(7, 0),
                    slice(15, 8),
                ]
            elif pn15_sel == 2:
                self.pn15_args = [
                    slice(23, 16),
                    slice(31, 24),
                    slice(15, 8),
                    slice(7, 0),
                ]
            else:
                self.pn15_args = [
                    slice(23, 16),
                    slice(31, 24),
                    slice(7, 0),
                    slice(15, 8),
                ]

        if not pn16_bps:
            # wire[1:0] pn16_sel = coef3[ 9: 8];
            pn16_sel = coef3[9:8]
            # wire[16:0] pn16_A = (pn16_sel == 0) ? addr[16:0] :
            #                     (pn16_sel == 1) ? addr[17:1] :
            #                     (pn16_sel == 2) ? addr[18:2] :
            #                                       addr[19:3];
            self.pn16_args = slice(16 + pn16_sel, pn16_sel)

        if not pn32_bps:
            # wire[1:0] pn32_sel = coef3[12:11];
            pn32_sel = coef3[12:11]
            # wire[31:0] pn32_A = (pn32_sel == 0) ?  addr[31:0] :
            #                     (pn32_sel == 1) ? {addr[ 7:0], addr[31: 8]} :
            #                     (pn32_sel == 2) ? {addr[15:0], addr[31:16]} :
            #                                       {addr[23:0], addr[31:24]};
            PN32_SHIFTS = (
                (0, 0),
                (2**8, 2**24),
                (2**16, 2**16),
                (2**24, 2**8),
            )
            self.pn32_args = PN32_SHIFTS[pn32_sel]

        # wire[31:0] random = rand_bps ? 32'h00000000 : coef2[31:0];
        self.random = 0 if rand_bps else coef2

    def encrypt_u32(self, addr: int, data: int) -> int:
        if self.bypass:
            return data
        addr = BitInt(addr)

        pn15_v = 0
        pn16_v = 0
        pn32_v = 0

        if self.pn15_args:
            pn15_a = (addr[self.pn15_args[0]] * 0x100) + addr[self.pn15_args[1]]
            pn15_b = (addr[self.pn15_args[2]] * 0x100) + addr[self.pn15_args[3]]
            pn15_A = pn15_a ^ pn15_b
            # wire[15:0] pn15_addr = pn15_A ^ coef1[31:16];
            pn15_addr = pn15_A ^ self.coef1_hi16
            pn15_v = pn15(pn15_addr)

        if self.pn16_args:
            pn16_A = addr[self.pn16_args]
            # wire[16:0] pn16_addr = pn16_A ^ {coef1[15:8], pn16_bit, coef1[7:0]};
            pn16_addr = pn16_A ^ self.coef1_mix
            pn16_v = pn16(pn16_addr)

        if self.pn32_args:
            pn32_A = (addr // self.pn32_args[0]) + (addr * self.pn32_args[1])
            # wire[31:0] pn32_addr = pn32_A ^ coef0[31:0];
            pn32_addr = pn32_A ^ self.coef0
            pn32_v = pn32(pn32_addr)

        # assign pnout = pn32[31:0] ^ {pn15[15:0], pn16[15:0]} ^ random[31:0];
        pnout = pn32_v ^ ((pn15_v * 0x10000) + (pn16_v % 0x10000)) ^ self.random
        return data ^ pnout

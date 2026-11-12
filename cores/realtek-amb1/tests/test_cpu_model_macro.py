#!/usr/bin/env python3
"""Host compile/run regression for SDK-first Ameba1 CPU model includes."""

import pathlib
import subprocess
import tempfile
import unittest


CORES = pathlib.Path(__file__).resolve().parents[2]
AMB1 = CORES / "realtek-amb1"


class CpuModelMacroTest(unittest.TestCase):
    def test_sdk_first_header_and_cpu_api(self):
        for compiler, standard, suffix in (
            ("cc", "c11", "c"),
            ("c++", "c++11", "cpp"),
        ):
            with self.subTest(compiler=compiler), tempfile.TemporaryDirectory() as tmp:
                directory = pathlib.Path(tmp)
                (directory / "libretiny.h").write_text(
                    '#include "lt_types.h"\n'
                    "void lt_get_device_mac(uint8_t *mac);\n"
                )
                (directory / "sdk_private.h").write_text(
                    "uint32_t HalGetCpuClk(void);\n"
                )
                source = directory / ("test." + suffix)
                source.write_text(
                    r'''
#include <assert.h>
#include <string.h>
/* Match AsyncTCP's SDK-before-Arduino include order. */
#include <platform_autoconf.h>
#include <lt_types.h>
#if RTL8195A != 1
#error SDK chip macro must remain intact
#endif
#include "lt_cpu.c"

void lt_get_device_mac(uint8_t *mac) {
    const uint8_t value[6] = {0, 1, 2, 0x12, 0x34, 0x56};
    memcpy(mac, value, sizeof(value));
}

uint32_t HalGetCpuClk(void) {
    return 166666666;
}

int main(void) {
    assert(CPU_RTL8195A == CPU_MODEL(F_RTL8710A, 0x00));
    assert(CPU_RTL8195A == 0x9F00);
    assert(RTL8711AM == CPU_RTL8195A);
    assert(lt_cpu_get_model() == CPU_RTL8195A);
    assert(lt_cpu_get_model() == RTL8711AM);
    assert(RTL8195A == 1);
    assert(strcmp(lt_cpu_get_core_type(), "ARM Cortex-M3 (ARMv7-M)") == 0);
    assert(lt_cpu_get_freq() == 166666666);
    assert(lt_cpu_get_mac_id() == 0x123456);
    return 0;
}
'''
                )
                executable = directory / "test"
                subprocess.run(
                    [
                        compiler,
                        "-std=" + standard,
                        "-Wall",
                        "-Wextra",
                        "-Werror",
                        "-I" + str(directory),
                        "-I" + str(CORES / "common" / "base"),
                        "-I" + str(AMB1 / "base" / "config"),
                        "-I" + str(AMB1 / "base" / "api"),
                        str(source),
                        "-o",
                        str(executable),
                    ],
                    check=True,
                    capture_output=True,
                    text=True,
                )
                subprocess.run([str(executable)], check=True)


if __name__ == "__main__":
    unittest.main()

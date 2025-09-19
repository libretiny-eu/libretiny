# Copyright (c) 2014-present PlatformIO <contact@platformio.org>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import re
import subprocess
import sys

from platformio.commands.device import DeviceMonitorFilter
from platformio.compat import PY2, WINDOWS, path_to_unicode
from platformio.project.exception import PlatformioException
from platformio.project.helpers import load_project_ide_data

# By design, __init__ is called inside miniterm and we can't pass context to it.
# pylint: disable=attribute-defined-outside-init


class RtlHardFaultDecoder(DeviceMonitorFilter):
    NAME = "rtl_hard_fault_decoder"

    def __call__(self):
        self.buffer = ""
        self.re = re.compile(r"^RTL8195A\[HAL\]: (LR|PC) = (0x[0-9a-fA-F]+)")

        self.firmware_path = None
        self.addr2line_path = None
        self.enabled = self.setup_paths()

        if self.config.get("env:" + self.environment, "build_type") != "debug":
            print(
                """
Please build project in debug configuration to get more details about an exception.
See https://docs.platformio.org/page/projectconf/build_configurations.html

"""
            )

        return self

    def setup_paths(self):
        self.project_dir = path_to_unicode(os.path.abspath(self.project_dir))
        try:
            data = load_project_ide_data(self.project_dir, self.environment)
            self.firmware_path = data["prog_path"]
            if not os.path.isfile(self.firmware_path):
                sys.stderr.write(
                    "%s: firmware at %s does not exist, rebuild the project?\n"
                    % (self.__class__.__name__, self.firmware_path)
                )
                return False

            cc_path = data.get("cc_path", "")
            if "-gcc" in cc_path:
                path = cc_path.replace("arm-none-eabi-gcc", "arm-none-eabi-addr2line")
                print(path)
                if os.path.isfile(path):
                    self.addr2line_path = path
                    return True
        except PlatformioException as e:
            sys.stderr.write(
                "%s: disabling, exception while looking for addr2line: %s\n"
                % (self.__class__.__name__, e)
            )
            return False
        sys.stderr.write(
            "%s: disabling, failed to find addr2line.\n" % self.__class__.__name__
        )
        return False

    def rx(self, text):
        if not self.enabled:
            return text

        last = 0
        while True:
            idx = text.find("\n", last)
            if idx == -1:
                if len(self.buffer) < 4096:
                    self.buffer += text[last:]
                break

            line = text[last:idx]
            if self.buffer:
                line = self.buffer + line
                self.buffer = ""
            last = idx + 1

            m = self.re.match(line.strip())
            if m is None:
                continue

            trace = self.addr2line(m.group(2))
            if trace:
                text = text[: idx + 1] + trace + text[idx + 1 :]
                last += len(trace)
        return text

    def addr2line(self, addr):
        trace = ""
        enc = "mbcs" if WINDOWS else "utf-8"
        args = [self.addr2line_path, "-fipC", "-e", self.firmware_path]
        if PY2:
            args = [a.encode(enc) for a in args]
        try:
            if PY2:
                addr = addr.encode(enc)
            output = subprocess.check_output(args + [addr]).decode(enc).strip()
            output = output.replace(
                "\n", "\n     "
            )  # newlines happen with inlined methods
            output = self.strip_project_dir(output)
            output = output.replace("\\", "/")
            trace += "  %s in %s\n" % (addr, output)
        except subprocess.CalledProcessError as e:
            sys.stderr.write(
                "%s: failed to call %s: %s\n"
                % (self.__class__.__name__, self.addr2line_path, e)
            )
        return trace

    def strip_project_dir(self, trace):
        while True:
            idx = trace.find(self.project_dir)
            if idx == -1:
                break
            trace = trace[:idx] + trace[idx + len(self.project_dir) + 1 :]
        return trace

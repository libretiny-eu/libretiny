# LibreTuya

<div align="center" markdown>

[![GitHub Workflow Status](https://img.shields.io/github/workflow/status/kuba2k2/libretuya/Deploy%20docs%20on%20GitHub%20Pages?label=docs&logo=markdown)](https://kuba2k2.github.io/libretuya/)
![GitHub last commit](https://img.shields.io/github/last-commit/kuba2k2/libretuya?logo=github)

[![Code style: clang-format](https://img.shields.io/badge/code%20style-clang--format-purple.svg)](.clang-format)
[![Code style: black](https://img.shields.io/badge/code%20style-black-000000.svg)](https://github.com/psf/black)

[![Discord](https://img.shields.io/discord/967863521511608370?color=%235865F2&label=Discord&logo=discord&logoColor=white)](https://discord.gg/SyGCB9Xwtf)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/kuba2k2/platform/libretuya.svg)](https://registry.platformio.org/platforms/kuba2k2/libretuya)

![RTL8710BN](https://img.shields.io/badge/-rtl8710bn-blue)
![BK7231](https://img.shields.io/badge/-bk7231-blue)

</div>

PlatformIO development platform for IoT modules manufactured by Tuya Inc.

The main goal of this project is to provide a usable build environment for IoT developers. While also providing vendor SDKs as PlatformIO cores,
the project focuses on developing working Arduino-compatible cores for supported families. The cores are inspired by Espressif's official core for ESP32,
which should make it easier to port/run existing ESP apps on Tuya IoT (and 3-rd party) modules.

LibreTuya also provides a common interface for all family implementations. The interface is based on ESP32 official libraries.

**Note:** this project is work-in-progress.

## Usage

1. [Install PlatformIO](https://platformio.org/platformio-ide)
2. `platformio platform install https://github.com/kuba2k2/libretuya`
3. Create a project, build it and upload!
4. See the [docs](https://kuba2k2.github.io/libretuya/) for any questions/problems.

## Board List

See [Boards & CPU list](https://kuba2k2.github.io/libretuya/docs/status/supported/).

## Arduino Core support status

Note: this list will probably change with each functionality update.

&nbsp;              | `realtek-ambz` | `beken-72xx`
--------------------|----------------|-------------
Core functions      | ✔️             | ✔️
GPIO/PWM/IRQ        | ✔️/✔️/✔️       | ✔️/✔️/✔️
Analog input (ADC)  | ✔️             | ✔️
Serial              | ✔️             | ✔️
Serial (extra)      | 0, 1, 2        | 1, 2
Flash I/O           | ✔️             | ✔️
**CORE LIBRARIES**  |                |
SoftwareSerial      | ✔️             | ❌
SPI                 | ❌              | ❌
Wire                | ❗              | ❌
**OTHER LIBRARIES** |                |
Wi-Fi STA/AP/Mixed  | ✔️             | ✔️
Wi-Fi Events        | ✔️             | ✔️
TCP Client (SSL)    | ✔️ (✔️)        | ✔️ (❗)
TCP Server          | ✔️             | ✔️
IPv6                | ❌              | ❌
HTTP Client (SSL)   | ✔️ (✔️)        | ❓
HTTP Server         | ✔️             | ✔️
NVS / Preferences   | ❌              | ❌
SPIFFS              | ❌              | ❌
BLE                 | -              | ❌
NTP                 | ❌              | ❌
OTA                 | ✔️             | ✔️
MDNS                | ✔️             | BK7231T only
MQTT                | ✅              | ❌
SD                  | ❌              | ❌

Symbols:

- ✔️ working
- ✅ tested, external library
- ❓ untested
- ❗ broken
- ❌ not implemented (yet?)
- \- not applicable

Names:

- Core functions - stuff like delay(), millis(), yield(), etc.
- **CORE LIBRARIES** - included normally in all Arduino cores
- **OTHER LIBRARIES** - included in ESP32 core or downloadable

## License

See [LICENSE](LICENSE). Project is licensed under MIT License.

Parts of the code may come from third parties, vendor SDKs or other open-source projects.
Most of these files are marked with appropriate copyright/author/license notices.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

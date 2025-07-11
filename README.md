# LibreTiny

<small>(formerly LibreTuya)</small>

<div align="center" markdown>

[![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/libretiny-eu/libretiny/push-master.yml?label=docs&logo=markdown)](https://docs.libretiny.eu/)
![GitHub last commit](https://img.shields.io/github/last-commit/libretiny-eu/libretiny?logo=github)

[![Code style: clang-format](https://img.shields.io/badge/code%20style-clang--format-purple.svg)](.clang-format)
[![Code style: black](https://img.shields.io/badge/code%20style-black-000000.svg)](https://github.com/psf/black)

[![Discord](https://img.shields.io/discord/967863521511608370?color=%235865F2&label=Discord&logo=discord&logoColor=white)](https://discord.gg/SyGCB9Xwtf)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/kuba2k2/platform/libretiny.svg)](https://registry.platformio.org/platforms/kuba2k2/libretiny)

![RTL8710BN](https://img.shields.io/badge/-rtl8710bn-blue)
![BK7231](https://img.shields.io/badge/-bk7231-blue)
![LN882H](https://img.shields.io/badge/-ln882h-blue)

</div>

PlatformIO development platform for BK7231, RTL8710 and LN882H IoT chips.

The main goal of this project is to provide a usable build environment for IoT developers. While also providing vendor SDKs as PlatformIO cores,
the project focuses on developing working Arduino-compatible cores for supported families. The cores are inspired by Espressif's official core for ESP32,
which should make it easier to port/run existing ESP apps on less-common, unsupported IoT modules.

**There's an [ESPHome port](https://docs.libretiny.eu/docs/projects/esphome/) based on LibreTiny, which supports BK7231 and RTL8710B chips.**

**Note:** this project is work-in-progress.

<div align="center" markdown>

## [⭐ Getting started ⭐](https://docs.libretiny.eu/docs/getting-started/)

</div>

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

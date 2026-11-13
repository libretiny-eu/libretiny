# realtek-amb1 (RTL8710A / RTL8195A / RTL8711AM) family builder.
#
# Adapted from builder/family/realtek-ambz.py. The framework-realtek-amb1
# package ships BOTH the 8711b (AmebaZ) and 8195a (Ameba1) SoC trees; the ambz
# family builds 8711b, this one builds 8195a. Differences from ambz:
#   - Cortex-M3, no FPU (vs M4F)
#   - CONFIG_PLATFORM_8195A (vs 8711B)
#   - single SDRAM image2 linker (vs xip1/xip2 split)
#   - FreeRTOS v8.1.2 (ARM_CM3) + LibreTiny's lwIP2 Ameba1 port
#
# Experimental port: compile success does not establish hardware connectivity.

from os.path import isdir, join
from pathlib import Path
from shutil import copyfile
import subprocess

from platformio.platform.board import PlatformBoardConfig
from SCons.Script import Builder, DefaultEnvironment, Environment


def stage_image2_sdk_lock(sdk_dir, build_dir, patch_path):
    """Patch disposable SDK copies, never the installed framework package."""
    stage = Path(build_dir).resolve() / "amb1_image2_sdk"
    for relative in (
        "component/os/os_dep/device_lock.c",
        "component/os/os_dep/include/device_lock.h",
    ):
        target = stage / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        copyfile(Path(sdk_dir) / relative, target)
        # SDK releases mix CRLF headers with LF sources; normalize only copies
        # so exact-context patching works without permitting fuzzy matches.
        target.write_bytes(target.read_bytes().replace(b"\r\n", b"\n"))
    result = subprocess.run(
        ["patch", "--batch", "--forward", "--fuzz=0", "-p1",
         "-i", str(Path(patch_path).resolve())],
        cwd=stage, capture_output=True, text=True,
    )
    if result.returncode or "fuzz" in (result.stdout + result.stderr).lower():
        raise RuntimeError(
            "AMB1 SDK lock patch failed (no fuzz permitted):\n"
            + result.stdout + result.stderr
        )
    return stage


def prepare_linker_script(env, board):
    """Stage the selected script before linker flags and framework hooks run."""
    selected = Path(env.subst(board.get("build.ldscript")))
    if selected.is_absolute():
        candidates = [selected]
    else:
        candidates = [
            Path(env.subst("$PROJECT_DIR")) / selected,
            Path(env.subst("$MISC_DIR")) / selected,
        ]
        candidates.extend(
            Path(env.subst(str(path))) / selected
            for path in env.get("LIBPATH", [])
        )
    source = next((path.resolve() for path in candidates if path.is_file()), None)
    if source is None:
        raise FileNotFoundError(f"Linker script not found: {selected}")

    target = Path(env.subst("$BUILD_DIR")).resolve() / source.name
    target.parent.mkdir(parents=True, exist_ok=True)
    # Refresh on every setup so patches do not accumulate between builds. Copy
    # contents only: a read-only framework package must yield a writable script.
    if source != target:
        copyfile(source, target)
    env.Replace(LDSCRIPT_PATH=str(target))
    board.update("build.ldscript", str(target))


env: Environment = DefaultEnvironment()
board: PlatformBoardConfig = env.BoardConfig()
queue = env.AddLibraryQueue("realtek-amb1")
env.ConfigureFamily()
prepare_linker_script(env, board)

# Make the 8195a SDK headers visible to ALL libraries — the SDK sources here AND
# LibreTiny's core/fixups (built on a separate queue in base.py). The Realtek SDK
# Makefile uses one global INCLUDES set; mirror that on the global CPPPATH.
_SDK_INCLUDES = [
    "component/soc/realtek/8195a/cmsis",
    "component/soc/realtek/8195a/cmsis/device",
    "component/soc/realtek/8195a/fwlib",
    "component/soc/realtek/8195a/fwlib/include",
    "component/soc/realtek/8195a/fwlib/rtl8195a",
    "component/soc/realtek/8195a/misc/driver",
    "component/soc/realtek/8195a/misc/os",
    "component/soc/realtek/8195a/misc/platform",
    "component/soc/realtek/common/bsp",
    "component/common/network/ssl/ssl_ram_map/rom",
    "component/common/mbed/api",
    "component/common/mbed/hal",
    "component/common/mbed/hal_ext",
    "component/common/mbed/targets/cmsis",
    "component/common/mbed/targets/hal/rtl8195a",
    "component/common/api",
    "component/common/api/platform",
    "component/common/api/wifi",
    "component/common/api/network/include",
    "component/common/network",
    "component/common/utilities",
    "component/common/api/wifi/rtw_wpa_supplicant/src",
    "component/common/api/wifi/rtw_wpa_supplicant/wpa_supplicant",
    "component/common/drivers/wlan/realtek/wlan_ram_map/rom",
    "component/common/drivers/wlan/realtek/include",
    "component/common/drivers/wlan/realtek/src/osdep",
    "component/common/network/ssl/mbedtls-2.4.0/include",
    "component/os/freertos",
    "component/os/os_dep/include",
]
# Resolve $SDK_DIR to its absolute path NOW. The amb1 family slot doesn't wire
# the framework package through board.get("package") the way ambz does, so
# SDK_DIR can come back empty; resolve the on-disk package directly and set it.
_SDK = env.subst("$SDK_DIR")
if not _SDK or not isdir(join(_SDK, "component")):
    _SDK = env.PioPlatform().get_package_dir("framework-realtek-amb1") or _SDK
    env.Replace(SDK_DIR=_SDK)
env.Append(CPPPATH=[join(_SDK, *p.split("/")) for p in _SDK_INCLUDES])

# Explicit qualification-build opt-in only. This does not bind a scratch policy
# or authorize runtime writes. Set flags globally before any queue clones env.
_IMAGE2_SDK_ADAPTER = str(env.GetProjectOption(
    "custom_amb1_flash_qualification", "no"
)).strip().lower() == "yes"
if _IMAGE2_SDK_ADAPTER:
    _OTA = env.subst("$CORES_DIR/realtek-amb1/base/ota")
    env.Prepend(CPPPATH=[_OTA])
    _STAGED_SDK = stage_image2_sdk_lock(
        _SDK, env.subst("$BUILD_DIR"),
        join(_OTA, "image2_controller_lock.patch"),
    )
    env.Prepend(CPPPATH=[str(_STAGED_SDK / "component/os/os_dep/include")])
    env.Append(CPPDEFINES=[("LT_AMB1_EXPERIMENTAL_IMAGE2_SDK_ADAPTER", 1)])
    queue.AddLibrary(
        name="amb1_image2_sdk_lock",
        base_dir=str(_STAGED_SDK),
        srcs=["+<component/os/os_dep/device_lock.c>"],
    )
    queue.AddLibrary(
        name="amb1_image2_sdk_adapter",
        base_dir=_OTA,
        srcs=["+<image2_sdk_adapter.c>"],
    )

# Flags — Cortex-M3, soft-float, Ameba-1 (RTL8195A)
queue.AppendPublic(
    CCFLAGS=[
        "-mcpu=cortex-m3",
        "-mthumb",
        "-fno-short-enums",
    ],
    CPPDEFINES=[
        "M3",
        "CONFIG_PLATFORM_8195A",
        ("ERRNO", "1"),
        # lwIP2 provides socklen_t; tell mbedTLS net_sockets.c to use it (else it
        # falls back to int* and mismatches lwip_getsockname/accept/... prototypes)
        "__socklen_t_defined",
    ],
    LINKFLAGS=[
        "-mcpu=cortex-m3",
        "-mthumb",
        "--specs=nano.specs",
        "-Wl,--entry=Reset_Handler",
        # Force-link the image2 entry table (gImage2EntryFun0) from the prebuilt
        # platform lib; it populates .image2.start.table (entry ptr + RTKWin sig)
        # that the ROM loader validates. --gc-sections would otherwise drop it.
        "-Wl,--undefined=gImage2EntryFun0",
        "-Wl,--undefined=_AppStart",
        # Match the Ameba1 SDK/OpenBeken ROM fixes used by the WLAN archive.
        "-Wl,--wrap=rom_psk_CalcPTK",
        "-Wl,--wrap=rom_psk_CalcGTK",
        "-Wl,--wrap=aes_80211_encrypt",
        "-Wl,--wrap=aes_80211_decrypt",
        # Temporary receive-task context diagnostic; no scheduler behavior change.
        "-Wl,--wrap=vTaskSwitchContext",
        "-Wl,--wrap=flash_write_word",
        "-Wl,--wrap=flash_stream_write",
        "-Wl,--wrap=flash_burst_write",
        "-Wl,--wrap=flash_erase_sector",
        "-Wl,--wrap=flash_erase_block",
        "-Wl,--wrap=flash_erase_chip",
        "-Wl,--wrap=flash_write_protect",
        "-Wl,--wrap=flash_set_status",
        "-Wl,--wrap=flash_reset_status",
        "-Wl,--wrap=flash_set_extend_addr",
        "-Wl,--gc-sections",
        # The linker script comes from board.build.ldscript via LDSCRIPT_PATH
        # (PlatformIO auto-adds -T). Do NOT add a second -T here — a double
        # inclusion redeclares the MEMORY regions and corrupts symbol addresses.
    ],
)
queue.AppendPrivate(
    CFLAGS=[
        "-Wno-implicit-function-declaration",
        "-Wno-incompatible-pointer-types",
        "-Wno-int-conversion",
        "-Wno-pointer-sign",
    ],
)

# SDK sources — 8195a subtree (self-contained, like the realtek_ameba1_va0 build)
queue.AddLibrary(
    name="amb1_sdk",
    base_dir="$SDK_DIR",
    srcs=[
        # SoC: system init, fwlib HAL, chip drivers, misc (console, os, ota)
        "+<component/soc/realtek/8195a/cmsis/device/system_8195a.c>",
        "+<component/soc/realtek/8195a/fwlib/src/*.c>",
        "+<component/soc/realtek/8195a/fwlib/rtl8195a/src/*.c>",
        "+<component/soc/realtek/8195a/misc/driver/*.c>",
        "+<component/soc/realtek/8195a/misc/os/*.c>",
        "+<component/soc/realtek/8195a/misc/platform/*.c>",
        # mbed-style peripheral HAL
        "+<component/common/mbed/targets/hal/rtl8195a/*.c>",
        "+<component/common/mbed/common/*.c>",
        # Platform glue (stdlib/printf patch)
        "+<component/common/api/platform/*.c>",
        "+<component/common/api/wifi/wifi_conf.c>",
        "+<component/common/api/wifi/wifi_util.c>",
        "+<component/common/api/wifi/wifi_promisc.c>",
        "+<component/common/api/wifi/rtw_wpa_supplicant/wpa_supplicant/wifi_eap_config.c>",
        "ARDUINO" not in env and "+<component/common/api/wifi/wifi_ind.c>",
        "+<component/common/drivers/wlan/realtek/src/osdep/lwip_intf.c>",
        "+<component/common/network/dhcp/dhcps.c>",
        # lwip_netconf.c is supplied by the core's lwIP2 adapter.
    ],
    includes=[
        "+<component/soc/realtek/8195a/cmsis>",
        "+<component/soc/realtek/8195a/cmsis/device>",
        "+<component/soc/realtek/8195a/fwlib>",
        "+<component/soc/realtek/8195a/fwlib/include>",
        "+<component/soc/realtek/8195a/fwlib/rtl8195a>",
        "+<component/common/network/ssl/mbedtls-2.4.0/include>",
        "+<component/soc/realtek/8195a/fwlib/rtl8195a/include>",
        "+<component/soc/realtek/8195a/misc/driver>",
        "+<component/soc/realtek/8195a/misc/os>",
        "+<component/soc/realtek/8195a/misc/platform>",
        "+<component/soc/realtek/common/bsp>",
        "+<component/common/mbed/api>",
        "+<component/common/mbed/hal>",
        "+<component/common/mbed/hal_ext>",
        "+<component/common/mbed/targets/cmsis>",
        "+<component/common/mbed/targets/hal/rtl8195a>",
        "+<component/common/api>",
        "+<component/common/api/platform>",
        "+<component/common/api/wifi>",
        "+<component/common/api/network/include>",
        "+<component/common/drivers/wlan/realtek/include>",
        "+<component/common/drivers/wlan/realtek/src/osdep>",
    ],
    options=dict(CFLAGS=["-w"]),
)

# OS glue — the SDK's cmsis_os / freertos_service / os_dep wrappers only. The
# FreeRTOS KERNEL and lwIP come from LibreTiny's external libs (lwIP 2.x + the
# amb1 Cortex-M3 port), matching ambz. The SDK-bundled FreeRTOS v8.1.2 kernel and
# lwIP 1.4.1 are NOT used — the LibreTiny core expects lwip2.
queue.AddLibrary(
    name="amb1_osglue",
    base_dir="$SDK_DIR",
    srcs=[
        "+<component/os/freertos/cmsis_os.c>",
        "+<component/os/freertos/freertos_service.c>",
        "+<component/os/os_dep/*.c>",
        _IMAGE2_SDK_ADAPTER and "-<component/os/os_dep/device_lock.c>",
    ],
    options=dict(CFLAGS=["-w"]),
)

# FreeRTOS kernel + amb1 (Cortex-M3) port, and lwIP — from LibreTiny externals
env.Replace(FREERTOS_PORT=env["FAMILY_NAME"], FREERTOS_PORT_DEFINE="REALTEK_AMB1")
queue.AddExternalLibrary("freertos")
queue.AddExternalLibrary("freertos-port")
queue.AddExternalLibrary("lwip", port="amb1")

# Arduino's secure clients and ESPHome API need the SDK TLS implementation.
queue.AddLibrary(
    name="amb1_mbedtls",
    base_dir="$SDK_DIR",
    srcs=[
        "+<component/common/network/ssl/mbedtls-2.4.0/library/*.c>",
        "-<component/common/network/ssl/mbedtls-2.4.0/library/net_sockets.c>",
    ],
    includes=["+<component/common/network/ssl/mbedtls-2.4.0/include>"],
)

# Prebuilt libs (GCC) + linker config
queue.AppendPublic(
    LIBPATH=[
        join(
            "$SDK_DIR",
            "component", "soc", "realtek", "8195a",
            "misc", "bsp", "lib", "common", "GCC",
        ),
    ],
    LIBS=[
        "_platform",
        "_wlan",
        "_wps",
        "_p2p",
        "_dct",
        "_rtlstd",
        "m",
        "c",
        "nosys",
        "gcc",
    ],
)

# No ltchiptool image backend exists for amb1. Emit only the small SRAM entry
# segment here, never a flat binary spanning the 512 MiB SRAM/SDRAM gap.
# Despite the legacy .uf2 filename this is NOT an installation image; both load
# segments require separately validated native image2 packaging.
env["UF2OTA"] = ["raw"]
env["BUILDERS"]["BuildUF2OTA"] = Builder(
    action=env.VerboseAction(
        "$OBJCOPY -j .image2.start.table -j .ram_image2.text "
        "-j .ram_image2.rodata -j .ram.data -O binary $SOURCE $TARGET",
        "Building diagnostic SRAM segment $TARGET (not flashable)",
    ),
)

# Build all queued libraries
queue.BuildLibraries()

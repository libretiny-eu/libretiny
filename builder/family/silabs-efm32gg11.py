# SPDX-License-Identifier: MSLA
# Copyright (c) Kuba Szczodrzyński 2022-3-13 (pattern adapted)
# Phase 1 builder for silabs-efm32gg11 — minimum surface to link.

from os.path import isdir, join

from SCons.Script import DefaultEnvironment, Environment

env: Environment = DefaultEnvironment()

# Family-level setup
queue = env.AddLibraryQueue("silabs-efm32gg11")
env.ConfigureFamily()

SDK_DIR = env.subst("$SDK_DIR")
if not isdir(SDK_DIR):
    raise RuntimeError(f"Gecko SDK not present at {SDK_DIR}")

GSDK = join(SDK_DIR, "gecko_sdk")
DEVICE_DIR = join(GSDK, "platform", "Device", "SiliconLabs", "EFM32GG11B")
EMLIB_DIR = join(GSDK, "platform", "emlib")
CMSIS_DIR = join(GSDK, "platform", "CMSIS", "Core")
COMMON_DIR = join(GSDK, "platform", "common")

# Compiler flags
env.Append(
    CPPDEFINES=[
        "EFM32GG11B820F2048GM64=1",
        "ARM_MATH_CM4",
        ("F_CPU", "72000000L"),
        "LT_HAS_FREERTOS=1",
        # EFM32GG11 (Series 1) is not in emlib's default RAMFUNC allowlist, so
        # the MSC flash write loop would run from flash and HANG mid-transfer on
        # a sustained write (e.g. staging a multi-KB OTA image). This forces the
        # MSC routines into the .ram section (placed in RAM + copied by startup;
        # see efm32gg11b820.ld). Bench-confirmed: without it an 85 KB FAL write
        # stalls inside MSC_LoadWriteData.
        "EM_MSC_RUN_FROM_RAM=1",
        # Redirect the GSDK startup's `bl __START` to LibreTiny's lt_main(),
        # which calls lt_init_family() + __libc_init_array() + main(). The
        # default is _start (newlib), which would skip lt_init_family — leaving
        # the chip on the uncalibrated HFRCO (~19 MHz) instead of 72 MHz HFXO+DPLL,
        # and Phase 1 framework=base sketches would observe wrong timing.
        # Mirrors the Reset_Handler -> lt_main routing used by beken-72xx and
        # lightning-ln882h (which patch Reset_Handler directly; the GSDK startup
        # exposes a __START hook so we redirect via preprocessor instead).
        ("__START", "lt_main"),
    ],
    CCFLAGS=[
        "-mcpu=cortex-m4",
        "-mthumb",
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16",
        "-Os",
        "-ffunction-sections",
        "-fdata-sections",
        "--specs=nano.specs",
        "--specs=nosys.specs",
    ],
    LINKFLAGS=[
        "-mcpu=cortex-m4",
        "-mthumb",
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16",
        "--specs=nano.specs",
        "--specs=nosys.specs",
        "-Wl,--gc-sections",
        "-Wl,-Map=" + join("$BUILD_DIR", "${PROGNAME}.map"),
        # Force extraction of objects from libcore_efm32gg11.a whose only
        # entry points are weak-default overrides. The linker resolves a weak
        # reference (or a weak def in an already-linked object) WITHOUT
        # searching archives, so these objects silently drop out:
        #   - lt_fault.c.o: vendor startup owns __Vectors with weak `b .`
        #     handlers — faults spun silently instead of dumping to UART.
        #   - lt_init.c.o: lt_init_family() is a weak hook declared in
        #     cores/common lt_init.h; ld nop'd the call in lt_main and the
        #     chip ran its whole first bench day on the 19 MHz reset HFRCO
        #     (DWT-measured) with no clock init at all.
        # One symbol per object suffices: extraction brings the whole .o.
        "-Wl,--undefined=HardFault_Handler",
        "-Wl,--undefined=lt_init_family",
        #   - wf200_netif.c.o (libsilabs-wifi-port.a): provides the strong
        #     lt_wf200_netif_input over the weak RX no-op in sl_wfx_host.c.o,
        #     but nothing references the netif API until the WiFi library
        #     exists — without extraction, every RX frame hit the no-op.
        #     Anchor on _create (strong-only, no weak twin that could satisfy
        #     the reference without extraction).
        "-Wl,--undefined=lt_wf200_netif_create",
        # NOTE: -T <ldscript> is added automatically by PIO's piobuild.py from
        # $LDSCRIPT_PATH (set by env_configure() from board.build.ldscript). The
        # filename is resolved via LIBPATH, which frameworks/base.py prepends
        # with $CORES_DIR/<family>/misc — so we do NOT pass -T here ourselves.
    ],
    CPPPATH=[
        join(DEVICE_DIR, "Include"),
        join(EMLIB_DIR, "inc"),
        join(CMSIS_DIR, "Include"),
        join(COMMON_DIR, "inc"),
        join("$FAMILY_DIR", "base"),
        # OTA metadata + CRC32 headers (lt_ota_meta.h / lt_crc32.h) used by the
        # family's base/api/lt_ota.c and base/port/fal_flash_efm32gg11_port.c.
        join("$FAMILY_DIR", "base", "ota"),
    ],
)

# Do NOT override LDSCRIPT_PATH here: env_configure() sets it from
# board.build.ldscript ("efm32gg11b820.ld") and frameworks/base.py prepends
# $CORES_DIR/<family>/misc to LIBPATH so the linker can find the script.

# Shared OTA logic (lt_ota_meta.c, lt_crc32.c) lives in base/ota/. AddCoreSources()
# only globs a fixed subdir set (api/, common/, port/, wiring/, ... — NOT ota/), so
# these would never compile into the family core; register them explicitly. The
# host-only RAM-flash shim (lt_meta_hostshim.c) is deliberately excluded — it's
# for the cores/silabs-efm32gg11/test/ota unit tests, not the device build.
queue.AddLibrary(
    name="silabs-ota",
    base_dir=join("$FAMILY_DIR", "base", "ota"),
    srcs=[
        "+<lt_ota_meta.c>",
        "+<lt_crc32.c>",
    ],
)

# Add SDK sources: minimum (vendor startup + system) — enough to link a C main().
queue.AddLibrary(
    name="silabs-device",
    base_dir=DEVICE_DIR,
    srcs=[
        "+<Source/system_efm32gg11b.c>",
        "+<Source/GCC/startup_efm32gg11b.S>",
    ],
)

# emlib core — keep minimal; will grow as features land in later tasks.
queue.AddLibrary(
    name="silabs-emlib-core",
    base_dir=EMLIB_DIR,
    srcs=[
        "+<src/em_system.c>",
        "+<src/em_core.c>",
        "+<src/em_assert.c>",
        "+<src/em_cmu.c>",
        "+<src/em_gpio.c>",
        "+<src/em_usart.c>",
        "+<src/em_emu.c>",
        "+<src/em_wdog.c>",
        "+<src/em_timer.c>",
        "+<src/em_msc.c>",
    ],
)

# FreeRTOS kernel — wired from the GSDK mirror.
# Confirmed for GSDK v4.5.0: kernel sources at this path (tasks.c, queue.c, etc.
# at the root; portable/ subtree per port). Older GSDK lines used a Source/
# subdir — re-verify if bumping past v4.5.x.
FREERTOS_DIR = join(GSDK, "util", "third_party", "freertos", "kernel")

queue.AddLibrary(
    name="freertos-kernel",
    base_dir=FREERTOS_DIR,
    srcs=[
        "+<*.c>",
        "+<portable/MemMang/heap_4.c>",
        "+<portable/GCC/ARM_CM4F/port.c>",
    ],
)

env.Append(
    CPPPATH=[
        join(FREERTOS_DIR, "include"),
        join(FREERTOS_DIR, "portable", "GCC", "ARM_CM4F"),
        join("$FAMILY_DIR", "base", "config"),  # for FreeRTOSConfig.h
    ]
)

# WF200 FMAC driver (Phase 2) — plaintext SDIO build.
# Bus selection is by compiled source list (external-libs.json picks
# bus/sl_wfx_bus_sdio.c); SL_WFX_USE_SDIO is defined for host-port code and
# GSDK-example parity, not consumed by the driver itself. Secure Link stays
# compiled out by NOT defining SL_WFX_USE_SECURE_LINK.
queue.AddExternalLibrary("wfx-fmac-driver")
env.Append(
    CPPDEFINES=[
        ("SL_WFX_USE_SDIO", "1"),
        # Driver default is 64 (sl_wfx_configuration_defaults.h); lt_sdio
        # hard-codes 512-byte block mode and the GSDK config uses 512 — host
        # and card must agree or CMD53 block transfers corrupt.
        ("SL_WFX_SDIO_BLOCK_SIZE", "512"),
        # Lower the byte-vs-block threshold so a 510-byte queue read (the one
        # reachable length in 509..511) is rounded to one 512-byte block by the
        # BUS LAYER, which then relocates the piggy-backed control word from
        # the end of the block. Without this, our host shim's mod-4 rounding
        # turns 510 into a block-mode read the bus layer doesn't know about,
        # and sl_wfx_receive_frame reads a garbage ctrl_reg at offset 508.
        ("SL_WFX_SDIO_BLOCK_MODE_THRESHOLD", "509"),
    ]
)

# Board PDS (WF200 RF front-end config) is pulled from the GSDK package's
# brd4321a set (= the WGM160P radio board), not duplicated into the repo.
# sl_wfx_host.c does `#include "sl_wfx_pds.h"`; this puts the Apache-2.0 brd4321a
# copy on the include path so it resolves without an in-tree copy.
env.Append(
    CPPPATH=[
        join(GSDK, "platform", "radio", "wifi", "wfx_fmac_driver", "pds", "brd4321a")
    ]
)

# lwIP 2.1.2 from the GSDK wrapper (BSD-3) + its FreeRTOS sys_arch port.
# Config comes from cores/silabs-efm32gg11/base/config/lwipopts.h, which is
# found FIRST on the include path (ConfigureFamily prepends family base/config
# after — i.e. ahead of — common base/config; these queue includes are only
# appended). The port's include/arch/cc.h is shadowed by our config/arch/cc.h
# for the same reason — see the comment there (errno strategy).
LWIP_DIR = join(GSDK, "util", "third_party", "lwip")
queue.AddLibrary(
    name="lwip",
    base_dir=LWIP_DIR,
    srcs=[
        "+<lwip/src/api/*.c>",
        "+<lwip/src/core/*.c>",
        "+<lwip/src/core/ipv4/*.c>",
        "+<lwip/src/netif/ethernet.c>",
        # httpd app (softAP concurrent-server bench, Task 8/9). fs.c textually
        # #includes the stock fsdata.c (HTTPD_FSDATA_FILE default) relative to
        # its own dir — so fsdata.c is NOT listed here. Served page is lwIP's
        # built-in /index.html from lwip/src/apps/http/fs/.
        "+<lwip/src/apps/http/httpd.c>",
        "+<lwip/src/apps/http/fs.c>",
        "+<lwip-contrib/ports/freertos/sys_arch.c>",
    ],
    includes=[
        "+<lwip/src/include>",
        "+<lwip-contrib/ports/freertos/include>",
    ],
)

# WF200 Secure Link (Phase 2): this WGM160P is OTP-provisioned in ENFORCED
# Secure Link mode — plaintext host messages are rejected
# (SL_WFX_ERROR_SECURELINK_WRONG_ENCRYPTION_STATE). Enabling the driver's
# secure-link path is gated on SL_CATALOG_WFX_SECURE_LINK_PRESENT, which
# sl_wfx_configuration_defaults.h turns into both SL_WFX_USE_SECURE_LINK and
# (via secure_link/sl_wfx_secure_link_configuration.h) SL_WFX_SLK_CURVE25519=1.
# Defining that one catalog token keeps the curve choice and the enable
# consistent with the way the GSDK intends. The secure_link/sl_wfx_secure_link.c
# translation unit is added to the driver source list in external-libs.json; the
# host crypto hooks + FreeRTOS renegotiation task live in base/wifi/.
env.Append(
    CPPDEFINES=[
        "SL_CATALOG_WFX_SECURE_LINK_PRESENT=1",
    ]
)

# Software-only mbedtls 3.5.0 (from the GSDK tree) for the Secure Link crypto:
# curve25519 ECDH + CTR_DRBG/entropy key exchange, HMAC-SHA512/256, AES-CCM.
# No PSA / Silabs accelerator plugins (those target SLC-generated projects);
# pure software on the M4F is ample for a one-time ECDH plus small per-frame CCM.
# Config is cores/silabs-efm32gg11/base/config/mbedtls/mbedtls_config.h. Rather
# than fight CPPDEFINES quote-escaping for MBEDTLS_CONFIG_FILE (the spawn here
# strips/garbles the embedded quotes), we shadow mbedtls's OWN default include:
# build_info.h does `#include "mbedtls/mbedtls_config.h"` when MBEDTLS_CONFIG_FILE
# is undefined, and base/config is FIRST on the include path (ConfigureFamily
# prepend), so that resolves to our file ahead of the GSDK's stock config. The
# library/ dir is on the include path for mbedtls's internal "..."-style headers
# (bignum_core.h, constant_time_internal.h, ...).
MBEDTLS_DIR = join(GSDK, "util", "third_party", "mbedtls")
env.Append(
    CPPPATH=[
        join(MBEDTLS_DIR, "include"),
        join(MBEDTLS_DIR, "library"),
    ],
)
queue.AddLibrary(
    name="mbedtls",
    base_dir=MBEDTLS_DIR,
    srcs=[
        # curve25519 ECDH + big numbers
        "+<library/ecdh.c>",
        "+<library/ecp.c>",
        "+<library/ecp_curves.c>",
        "+<library/bignum.c>",
        "+<library/bignum_core.c>",
        # hashes + MAC
        "+<library/md.c>",
        "+<library/sha256.c>",
        "+<library/sha512.c>",
        # AES-CCM (CCM_C needs the cipher layer in mbedtls 3.x)
        "+<library/aes.c>",
        "+<library/cipher.c>",
        "+<library/cipher_wrap.c>",
        "+<library/ccm.c>",
        # RNG
        "+<library/ctr_drbg.c>",
        "+<library/entropy.c>",
        "+<library/entropy_poll.c>",
        # platform shims
        "+<library/platform.c>",
        "+<library/platform_util.c>",
        "+<library/constant_time.c>",
    ],
    includes=[
        "+<include>",
        "+<library>",
    ],
)

# base/wifi/ (WF200 host port) is NOT auto-discovered: cores.py's
# AddCoreSources() globs api/, common/, compat/, port/, posix/, wiring/,
# wraps/ (and ConfigureFamily() only adds fixups/config/compat include
# paths), so the wifi/ subdir must be wired here explicitly.
queue.AddLibrary(
    name="silabs-wifi-port",
    base_dir=join("$FAMILY_DIR", "base"),
    srcs=[
        "+<wifi/*.c*>",
    ],
    includes=[
        # compat/ FIRST so our app_webpage.h shim shadows the GSDK example's
        # (which lives one dir up from the apps/ dir we add below — and which we
        # deliberately do NOT put on the path). The GSDK dhcp_server.c picks up
        # this shim's app_webpage.h, not the example's webpage-coupled header.
        "+<wifi/compat>",
        "+<wifi>",
        # lt_sdio.h: the core_* libraries' include paths live in base.py's
        # queue (built after this one), so port/ must be re-added here.
        # NOTE: <libretiny.h> can NOT be used by this library — the family
        # queue builds before base.py appends the public defines it needs.
        "+<port>",
    ],
)

# AP DHCP server: reuse the GSDK Apache-2.0 dhcp_server.c UNMODIFIED from the
# framework package (not vendored into this repo). Its own library (its base_dir
# is the GSDK apps/ dir). It #include "app_webpage.h" but uses only the AP
# IP/netmask octet globals + the global `ap_netif` from it; the
# wifi/compat/app_webpage.h shim added to the wifi-port include path above (and
# thus on the global CPPPATH once this queue builds) shadows the example's
# header so none of the Micrium/webpage example deps come along. The lt_dhcpserver.c
# glue (in wifi/*.c above) defines the octet globals and calls dhcpserver_start/stop.
DHCP_SERVER_APPS_DIR = join(
    GSDK,
    "app",
    "common",
    "example",
    "wifi_commissioning_micriumos",
    "lwip_host",
    "apps",
)
queue.AddLibrary(
    name="silabs-dhcp-server",
    base_dir=DHCP_SERVER_APPS_DIR,
    srcs=[
        "+<dhcp_server.c>",
    ],
    includes=[
        # dhcp_server.h lives next to dhcp_server.c; expose ONLY apps/ (NOT its
        # parent, which holds the real app_webpage.h we are shadowing).
        "+<.>",
    ],
)

# Note: cores/silabs-efm32gg11/base/ sources (api/*.c, port/*.c, fixups/*.c)
# are added automatically by frameworks/base.py via env.AddCoreSources() over
# family.inheritance. We do NOT re-add them here — doing so would compile each
# file twice and produce duplicate-symbol link errors.
#
# Same for cores/silabs-efm32gg11/arduino/{src,libraries}/ when framework=arduino:
# frameworks/arduino.py handles those via the equivalent mechanism. Family
# builders only add SDK sources and any special fixup libraries here.

# Linker: add nosys to LIBS so libg_nano.a's abort()/exit() paths can resolve
# their references to _exit/_kill/_getpid/etc. against newlib's nosys stubs.
# Our cores/silabs-efm32gg11/base/fixups/syscalls.c provides strong overrides
# (notably _exit -> NVIC_SystemReset) — strong symbols defeat the weak nosys
# defaults, but nosys backstops the rest of the newlib syscall surface (sbrk,
# read, write, fstat, ...). Same pattern used by beken-72xx, lightning-ln882h,
# realtek-ambz, realtek-ambz2.
queue.AppendPublic(
    LIBS=["nosys"],
)

queue.BuildLibraries()

# ----------------------------------------------------------------------------
# Phase 1 post-link wiring
#
# main.py's BuildProgram() produces ${BUILD_DIR}/raw_firmware.elf. We need:
#   - firmware.bin: raw flash image (Commander/J-Link flashes this directly)
#   - firmware.elf: same as raw_firmware.elf, renamed so the size dispatcher's
#       `arm-none-eabi-size .../firmware.elf` finds it
#   - firmware.uf2: stub copy of firmware.bin so main.py's downstream targets
#       (which Depends() on this path) resolve. Phase 1 has no proper UF2
#       packaging — ltchiptool's UF2 writer requires a per-family SocInterface
#       module (a hardcoded if-chain in ltchiptool/soc/interface.py: bk72xx /
#       ambz / ambz2 / ln882h) which doesn't yet exist for EFM32. Adding one
#       is upstream-ltchiptool work, not in Phase 1 scope. Phase 2 will write
#       a proper SocInterface and PR it to libretiny-eu/ltchiptool.
# ----------------------------------------------------------------------------
firmware_elf = "${BUILD_DIR}/firmware.elf"
firmware_bin = "${BUILD_DIR}/firmware.bin"

# The linker script reaches the link line as a bare filename (-T <name>)
# resolved through LIBPATH, so SCons's dependency scanner never sees the
# actual file — editing the .ld did not trigger a relink, and the stale ELF
# (plus the firmware.bin/elf copies, whose post-actions only fire on relink)
# survived rebuilds until raw_firmware.elf was deleted by hand. Bench-bitten
# during 2026-06-10 bring-up. Declare the dependency explicitly.
env.Depends(
    "${BUILD_DIR}/${PROGNAME}.elf",
    join("$MISC_DIR", env.BoardConfig().get("build.ldscript")),
)

# firmware.bin / firmware.elf are real SCons targets, NOT AddPostAction hooks
# on the ELF. Post-actions only fire when SCons happens to re-execute the ELF
# node's own action and proved unreliable on dependency-driven relinks during
# 2026-06-10 bring-up (raw_firmware.elf relinked, firmware.bin stayed stale —
# flashing the stale copy cost a debug cycle). As Command targets, SCons
# rebuilds them whenever the ELF changes or the copies are missing.
target_fw_bin = env.Command(
    firmware_bin,
    "${BUILD_DIR}/${PROGNAME}.elf",
    env.VerboseAction(
        "$OBJCOPY -O binary $SOURCE $TARGET",
        "Producing firmware.bin",
    ),
)
target_fw_elf = env.Command(
    firmware_elf,
    "${BUILD_DIR}/${PROGNAME}.elf",
    env.VerboseAction("cp $SOURCE $TARGET", "Producing firmware.elf"),
)
# Pull both into the default build graph via the firmware.uf2 node (built by
# main.py from the ELF; its stub action also copies firmware.bin, so the
# dependency is real, not just a graph trick).
env.Depends("${BUILD_DIR}/firmware.uf2", target_fw_bin + target_fw_elf)

# ----------------------------------------------------------------------------
# OTA dual-bank link (Task OTA T4)
#
# OTA links the SAME sources at two independent bank bases:
#   - bank A @ 0x008000 (this is the DEFAULT build: raw_firmware.elf already
#     links here via efm32gg11b820.ld's __flash_origin default)
#   - bank B @ 0x100000 (a second link of the SAME object files)
#
# Mechanism: the linker script reads FLASH ORIGIN/LENGTH from defsym symbols
# (__flash_origin / __flash_length) with bank-A defaults. arm-none-eabi-ld
# accepts `ORIGIN = <symbol>` in a MEMORY block (link-verified), so no
# per-bank generated .ld is needed — bank B is produced by re-linking with
# -Wl,--defsym overriding those symbols. (realtek-ambz needs the
# template-substitution route because its template embeds literal arithmetic
# the assembler-style defsym can't express; this toolchain does not.)
#
# We do NOT append CRC/length to the bins here — those live in the OTA
# metadata page and are computed at switch time (see lt_ota_meta.*).
#
# firmware_a.bin is just the bank-A firmware.bin under the OTA-conventional
# name (a later bench task consumes firmware_a.bin / firmware_b.bin).
firmware_a_bin = "${BUILD_DIR}/firmware_a.bin"
firmware_b_elf = "${BUILD_DIR}/firmware_b.elf"
firmware_b_bin = "${BUILD_DIR}/firmware_b.bin"

# Bank A: copy of the default (bank-A-linked) firmware.bin.
target_fw_a_bin = env.Command(
    firmware_a_bin,
    firmware_bin,
    env.VerboseAction("cp $SOURCE $TARGET", "Producing firmware_a.bin (bank A)"),
)


# Bank B: re-link the SAME objects at 0x100000. The object files are the
# children of the bank-A ELF node; reuse the env's standard link command with
# the defsym override prepended to LINKFLAGS. Resolved at action time so the
# full (possibly framework-extended) object/lib list is captured.
def _link_bank_b(target, source, env):
    import subprocess

    elf_node = env.File("${BUILD_DIR}/${PROGNAME}.elf")
    objs = [str(c) for c in elf_node.children() if str(c).endswith(".o")]
    out = str(target[0])
    link = env.subst("$LINK")
    # Drop the bank-A -Map flag: it carries the raw_firmware.map path (and the
    # subst-then-split mangles its embedded value), and we don't want bank B to
    # clobber the bank-A map. Re-point it at a bank-B map instead.
    flags = [f for f in env.subst("$LINKFLAGS").split() if "-Wl,-Map=" not in f]
    flags.append("-Wl,-Map=" + env.subst("${BUILD_DIR}/firmware_b.map"))
    libdirs = env.subst("$_LIBDIRFLAGS").split()
    libflags = env.subst("$_LIBFLAGS").split()
    cmd = (
        [link]
        + flags
        + [
            "-Wl,--defsym=__flash_origin=0x100000",
            "-Wl,--defsym=__flash_length=0x000F0000",
        ]
        + ["-o", out]
        + objs
        + libdirs
        + libflags
    )
    return subprocess.call(cmd)


target_fw_b_elf = env.Command(
    firmware_b_elf,
    "${BUILD_DIR}/${PROGNAME}.elf",
    env.VerboseAction(_link_bank_b, "Linking firmware_b.elf (bank B @ 0x100000)"),
)
# The bank-B link consumes the SAME .ld (with the defsym override); declare the
# dependency explicitly — same stale-relink rationale as the bank-A ELF above.
env.Depends(
    target_fw_b_elf,
    join("$MISC_DIR", env.BoardConfig().get("build.ldscript")),
)
target_fw_b_bin = env.Command(
    firmware_b_bin,
    firmware_b_elf,
    env.VerboseAction("$OBJCOPY -O binary $SOURCE $TARGET", "Producing firmware_b.bin"),
)
env.Depends(
    "${BUILD_DIR}/firmware.uf2",
    target_fw_a_bin + target_fw_b_elf + target_fw_b_bin,
)

# ----------------------------------------------------------------------------
# MIT first-stage bootloader (OTA T5) @ flash 0x0, 32 KB.
#
# Built STANDALONE — a single arm-none-eabi-gcc compile+link invocation over a
# handful of small C sources — NOT threaded through the app's SCons link env
# (which carries FreeRTOS/Arduino/lwIP/mbedtls). The bootloader shares the SAME
# OTA modules (lt_ota_meta.c / lt_crc32.c) as the app, so the on-flash metadata
# format is byte-identical. -D__START=main routes the GSDK startup's
# `bl __START` straight into the bootloader's main() after SystemInit + .data/.bss
# init (no newlib _start / libc_init_array).
# ----------------------------------------------------------------------------
BOOTLOADER_DIR = join("$FAMILY_DIR", "bootloader")
OTA_DIR = join("$FAMILY_DIR", "base", "ota")
bootloader_elf = "${BUILD_DIR}/bootloader.elf"
bootloader_bin = "${BUILD_DIR}/bootloader.bin"


def _build_bootloader(target, source, env):
    import subprocess

    cc = env.subst("$CC")
    objcopy = env.subst("$OBJCOPY")
    out_elf = env.subst(bootloader_elf)
    out_bin = str(target[0])
    ld = env.subst(join(BOOTLOADER_DIR, "bootloader.ld"))

    arch = [
        "-mcpu=cortex-m4",
        "-mthumb",
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16",
    ]
    cflags = arch + [
        "-Os",
        "-ffunction-sections",
        "-fdata-sections",
        "--specs=nano.specs",
        "--specs=nosys.specs",
    ]
    defines = [
        "-DEFM32GG11B820F2048GM64=1",
        "-D__START=main",
        # Run emlib MSC from RAM (see bootloader.ld .ram section): the bootloader
        # rewrites the OTA metadata page on every trial boot, and the GG11 MSC
        # write loop hangs if executed from flash.
        "-DEM_MSC_RUN_FROM_RAM=1",
    ]
    incs = [
        "-I" + env.subst(join(DEVICE_DIR, "Include")),
        "-I" + env.subst(join(EMLIB_DIR, "inc")),
        "-I" + env.subst(join(CMSIS_DIR, "Include")),
        # emlib sources #include "sl_common.h"/"sl_assert.h" from platform/common.
        "-I" + env.subst(join(COMMON_DIR, "inc")),
        "-I" + env.subst(OTA_DIR),
        "-I" + env.subst(join("$FAMILY_DIR", "base")),
    ]
    srcs = [
        env.subst(join(BOOTLOADER_DIR, "main.c")),
        env.subst(join(OTA_DIR, "lt_ota_meta.c")),
        env.subst(join(OTA_DIR, "lt_crc32.c")),
        env.subst(join(DEVICE_DIR, "Source", "system_efm32gg11b.c")),
        env.subst(join(DEVICE_DIR, "Source", "GCC", "startup_efm32gg11b.S")),
        env.subst(join(EMLIB_DIR, "src", "em_msc.c")),
        env.subst(join(EMLIB_DIR, "src", "em_core.c")),
        env.subst(join(EMLIB_DIR, "src", "em_system.c")),
    ]
    link = arch + ["-Wl,--gc-sections", "-T", ld]
    cmd = [cc] + cflags + defines + incs + srcs + link + ["-o", out_elf]
    rc = subprocess.call(cmd)
    if rc != 0:
        return rc
    return subprocess.call([objcopy, "-O", "binary", out_elf, out_bin])


target_bootloader_bin = env.Command(
    bootloader_bin,
    [
        join(BOOTLOADER_DIR, "main.c"),
        join(BOOTLOADER_DIR, "bootloader.ld"),
        join(OTA_DIR, "lt_ota_meta.c"),
        join(OTA_DIR, "lt_crc32.c"),
    ],
    env.VerboseAction(_build_bootloader, "Building bootloader.bin (MIT FSBL @ 0x0)"),
)
# Pull the bootloader into the default build graph alongside the bank artifacts.
env.Depends("${BUILD_DIR}/firmware.uf2", target_bootloader_bin)


# Override main.py's BuildUF2OTA for Phase 1: skip the ltchiptool UF2 packer
# (no SocInterface yet — see comment block above) and just copy firmware.bin to
# firmware.uf2 so downstream Depends() resolves.
def _phase1_uf2_stub(env, *args, **kwargs):
    import shutil

    print("|-- firmware.uf2 (Phase 1 stub: copy of firmware.bin; proper UF2")
    print("|   packaging deferred until ltchiptool SoC plugin lands)")
    shutil.copy(env.subst(firmware_bin), env.subst("${BUILD_DIR}/firmware.uf2"))


from SCons.Script import Builder  # noqa: E402

env.Append(
    BUILDERS=dict(
        BuildUF2OTA=Builder(
            action=[env.VerboseAction(_phase1_uf2_stub, "Phase 1: UF2 stub")]
        )
    )
)

env.Replace(
    # Placeholder; main.py still references env["UF2OTA"] for upload logic.
    UF2OTA=[
        f"{firmware_bin}=flasher:app",
    ],
)

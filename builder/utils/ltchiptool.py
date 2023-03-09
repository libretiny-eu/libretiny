# Copyright (c) Kuba Szczodrzyński 2022-06-02.

from datetime import datetime
from os.path import basename, join, normpath

from platformio.platform.base import PlatformBase
from SCons.Script import Builder, DefaultEnvironment, Environment

env: Environment = DefaultEnvironment()
platform: PlatformBase = env.PioPlatform()


def env_uf2ota(env: Environment, *args, **kwargs):
    now = datetime.now()
    project_dir = env.subst("$PROJECT_DIR")
    project_name = basename(normpath(project_dir))
    project_version = now.strftime("%y.%m.%d")
    lt_version = platform.version

    if platform.custom("fw_name"):
        project_name = platform.custom("fw_name")
    if platform.custom("fw_version"):
        project_version = platform.custom("fw_version")

    output = [
        project_name,
        project_version,
        "${VARIANT}",
        "${FAMILY}",
        f"lt{lt_version}",
    ]
    output = "_".join(output) + ".uf2"
    if platform.custom("fw_output"):
        output = platform.custom("fw_output")

    output = join("${BUILD_DIR}", output)
    output_copy_1 = join("${BUILD_DIR}", "firmware.uf2")
    output_copy_2 = join("${BUILD_DIR}", "firmware.bin")

    env["UF2OUT"] = output
    env["UF2OUT_BASE"] = basename(output)

    cmd = [
        "@${LTCHIPTOOL} uf2 write",
        f'--output "{output}"',
        f'--output-copy "{output_copy_1}"',
        f'--output-copy "{output_copy_2}"',
        "--family ${FAMILY}",
        "--board ${VARIANT}",
        f"--lt-version {lt_version}",
        f'--fw "{project_name}:{project_version}"',
        f"--date {int(now.timestamp())}",
        *env["UF2OTA"],
    ]

    print(f"|-- {basename(env.subst(output))}")
    env.Execute(" ".join(cmd))
    print(f"|-- {basename(env.subst(output_copy_1))}")
    print(f"|-- {basename(env.subst(output_copy_2))}")


def env_flash_write(env: Environment):
    protocol = env.subst("${UPLOAD_PROTOCOL}")
    if protocol == "uart":
        # upload via UART
        return [
            "-d",
            "${UPLOAD_PORT}",
            "-b",
            "${UPLOAD_SPEED}",
        ]
    else:
        # can't upload via ltchiptool
        return []


env.Append(
    BUILDERS=dict(
        BuildUF2OTA=Builder(
            action=[env.VerboseAction(env_uf2ota, "Building UF2 OTA image")]
        )
    )
)
env.AddMethod(env_flash_write, "GetLtchiptoolWriteFlags")

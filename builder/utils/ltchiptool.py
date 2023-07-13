# Copyright (c) Kuba Szczodrzyński 2022-06-02.

import json
from datetime import datetime
from os.path import basename, join, normpath

from platformio.platform.base import PlatformBase
from platformio.platform.board import PlatformBoardConfig
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
        "${MCULC}",
        f"lt{lt_version}",
    ]
    output = "_".join(output) + ".uf2"
    if platform.custom("fw_output"):
        output = platform.custom("fw_output")

    outputs = [
        join("${BUILD_DIR}", output),
        join("${BUILD_DIR}", "firmware.uf2"),
        join("${BUILD_DIR}", "firmware.bin"),
    ]
    output_opts = [f'--output "{output}"' for output in outputs]

    cmd = [
        "@${LTCHIPTOOL} uf2 write",
        *output_opts,
        "--board ${BOARD_JSON}",
        f"--lt-version {lt_version}",
        f'--fw "{project_name}:{project_version}"',
        f"--date {int(now.timestamp())}",
        "--legacy",
        *(f'"{arg}"' for arg in env["UF2OTA"]),
    ]

    for output in outputs:
        print(f"|-- {basename(env.subst(output))}")
    env.Execute(" ".join(cmd))


def env_flash_write(env: Environment):
    # UPLOAD_PROTOCOL = upload_protocol or board->upload.protocol
    # UPLOAD_PORT = upload_port (PIO can choose this automatically I guess)
    # UPLOAD_SPEED = upload_speed or board->upload.speed (**can be empty**)
    protocol = env.subst("${UPLOAD_PROTOCOL}")
    speed = env.subst("${UPLOAD_SPEED}")
    if protocol == "uart":
        # upload via UART
        if speed:
            return [
                "-d",
                "${UPLOAD_PORT}",
                "-b",
                "${UPLOAD_SPEED}",
            ]
        return [
            "-d",
            "${UPLOAD_PORT}",
        ]
    else:
        # can't upload via ltchiptool
        return []


def env_export_board_data(env: Environment, board: PlatformBoardConfig):
    output = join("${BUILD_DIR}", "board.json")
    with open(env.subst(output), "w") as f:
        json.dump(board.manifest, f, indent="\t")
    env["BOARD_JSON"] = output


env.Append(
    BUILDERS=dict(
        BuildUF2OTA=Builder(
            action=[env.VerboseAction(env_uf2ota, "Building UF2 OTA image")]
        )
    )
)
env.AddMethod(env_flash_write, "GetLtchiptoolWriteFlags")
env.AddMethod(env_export_board_data, "ExportBoardData")

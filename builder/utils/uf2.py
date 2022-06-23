# Copyright (c) Kuba Szczodrzyński 2022-06-02.

import sys
from datetime import datetime
from os.path import basename, join, normpath

from SCons.Script import Builder, DefaultEnvironment

env = DefaultEnvironment()
platform = env.PioPlatform()


def env_uf2ota(env, *args, **kwargs):
    now = datetime.now()
    project_dir = env.subst("$PROJECT_DIR")
    project_name = basename(normpath(project_dir))
    project_version = now.strftime("%y.%m.%d")
    lt_version = platform.version

    if platform.custom("fw_name"):
        project_name = platform.custom("fw_name")
    if platform.custom("fw_version"):
        project_version = platform.custom("fw_version")

    inputs = " ".join(f'"{";".join(input)}"' for input in env["UF2OTA"])
    output = [
        project_name,
        project_version,
        "${VARIANT}",
        "${FAMILY}",
        f"lt{lt_version}",
    ]
    output = join("${BUILD_DIR}", "_".join(output)) + ".uf2"
    env["UF2OUT"] = output
    env["UF2OUT_BASE"] = basename(output)

    cmd = [
        "@${UF2OTA_PY}",
        f'--output "{output}"',
        "--family ${FAMILY}",
        "--board ${VARIANT}",
        f"--version {lt_version}",
        f'--fw "{project_name}:{project_version}"',
        f"--date {int(now.timestamp())}",
        "write",
        inputs,
    ]

    print(f"|-- {basename(env.subst(output))}")

    env.Execute(" ".join(cmd))


def env_uf2upload(env, target):
    protocol = env.subst("${UPLOAD_PROTOCOL}")
    actions = []
    # from platform-espressif32/builder/main.py
    if protocol == "uart":
        # upload via UART
        env["UPLOADERFLAGS"] = [
            "${UF2OUT}",
            "uart",
            "${UPLOAD_PORT}",
            "-b",
            "${UPLOAD_SPEED}",
        ]
        actions = [
            env.VerboseAction(env.AutodetectUploadPort, "Looking for upload port..."),
        ]
    elif protocol == "custom":
        actions = [
            env.VerboseAction("${UPLOADCMD}", "Uploading firmware"),
        ]
    else:
        sys.stderr.write("Warning! Unknown upload protocol %s\n" % protocol)
        return

    # add main upload target
    env.Replace(UPLOADER="${UF2UPLOAD_PY}", UPLOADCMD="${UPLOADER} ${UPLOADERFLAGS}")
    actions.append(env.VerboseAction("${UPLOADCMD}", "Uploading ${UF2OUT_BASE}"))
    env.AddPlatformTarget("upload", target, actions, "Upload")


env.Append(
    BUILDERS=dict(
        BuildUF2OTA=Builder(
            action=[env.VerboseAction(env_uf2ota, "Building UF2 OTA image")]
        )
    )
)
env.AddMethod(env_uf2upload, "AddUF2Uploader")

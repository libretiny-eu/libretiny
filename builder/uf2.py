# Copyright (c) Kuba Szczodrzyński 2022-06-02.

from datetime import datetime
from os.path import basename, join, normpath

from SCons.Script import Builder, DefaultEnvironment

env = DefaultEnvironment()
platform = env.PioPlatform()


def env_uf2ota(env, *args, **kwargs):
    now = datetime.now()
    project_dir = env.subst("$PROJECT_DIR")
    project_name = basename(normpath(project_dir))
    # TODO support specifying custom version
    project_version = now.strftime("%y.%m.%d")
    lt_version = platform.version

    inputs = " ".join(f'"{";".join(input)}"' for input in env["UF2OTA"])
    output = [
        project_name,
        project_version,
        "${VARIANT}",
        "${FAMILY}",
        f"lt{lt_version}",
    ]
    output = join("${BUILD_DIR}", "_".join(output)) + ".uf2"

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


env.Append(
    BUILDERS=dict(
        BuildUF2OTA=Builder(
            action=[env.VerboseAction(env_uf2ota, "Building UF2 OTA image")]
        )
    )
)

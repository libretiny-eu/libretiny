# Copyright (c) Kuba Szczodrzyński 2023-09-07.

import json
import shutil
import site
import subprocess
import sys
from pathlib import Path

import semantic_version
from platformio.compat import IS_WINDOWS
from platformio.package.version import pepver_to_semver
from platformio.platform.base import PlatformBase
from SCons.Script import DefaultEnvironment, Environment

env: Environment = DefaultEnvironment()
platform: PlatformBase = env.PioPlatform()

# code borrowed and modified from espressif32/builder/frameworks/espidf.py


def env_configure_python_venv(env: Environment):
    venv_path = Path(env.subst("${PROJECT_CORE_DIR}"), "penv", ".libretiny")

    pip_path = venv_path.joinpath(
        "Scripts" if IS_WINDOWS else "bin",
        "pip" + (".exe" if IS_WINDOWS else ""),
    )
    python_path = venv_path.joinpath(
        "Scripts" if IS_WINDOWS else "bin",
        "python" + (".exe" if IS_WINDOWS else ""),
    )
    site_path = venv_path.joinpath(
        "Lib" if IS_WINDOWS else "lib",
        "." if IS_WINDOWS else f"python{sys.version_info[0]}.{sys.version_info[1]}",
        "site-packages",
    )

    # uv (much faster than pip) is used when available, e.g. in the ESPHome docker image
    uv_path = shutil.which("uv")

    if not (pip_path.is_file() or (uv_path and python_path.is_file())):
        # Use the built-in PlatformIO Python to create a standalone virtual env
        python_str = env["PYTHONEXE"]
        venv_str = str(venv_path.absolute())
        if uv_path:
            venv_cmd = [uv_path, "venv", "--python", python_str, "--clear", venv_str]
        else:
            venv_cmd = [python_str, "-m", "venv", "--clear", venv_str]

        try:
            env.RunCommand(
                *venv_cmd,
                message="LibreTiny: Creating a virtual environment for Python dependencies",
            )
        except Exception as e:
            env.PrintCommandError(e, message="Failed to create virtual environment")
            raise SystemExit(1)

        if not (pip_path.is_file() or uv_path):
            # Creating the venv succeeded but pip didn't get installed,
            # and uv is not available
            # (i.e. Debian/Ubuntu without ensurepip)
            print(
                "LibreTiny: Failed to install pip, running get-pip.py", file=sys.stderr
            )
            import requests

            with requests.get("https://bootstrap.pypa.io/get-pip.py") as r:
                p = subprocess.Popen(
                    args=str(python_path.absolute()),
                    stdin=subprocess.PIPE,
                )
                p.communicate(r.content)
                p.wait()

    assert (
        pip_path.is_file() or uv_path
    ), f"Error: Missing the pip binary in virtual environment `{pip_path.absolute()}`"
    assert (
        python_path.is_file()
    ), f"Error: Missing Python executable file `{python_path.absolute()}`"
    assert (
        site_path.is_dir()
    ), f"Error: Missing site-packages directory `{site_path.absolute()}`"

    python_str = str(python_path.absolute())
    if uv_path:
        pip_cmd = lambda *args: [uv_path, "pip", *args, "--python", python_str]
    else:
        pip_cmd = lambda *args: [
            python_str,
            "-m",
            "pip",
            "--disable-pip-version-check",
            *args,
        ]

    env.Replace(
        LTPYTHONEXE=python_path.absolute(),
        LTPYTHONENV=venv_path.absolute(),
        LTPIPCMD=pip_cmd,
    )
    site.addsitedir(str(site_path.absolute()))


def env_install_python_dependencies(env: Environment, dependencies: dict):
    pip_cmd = env["LTPIPCMD"]
    try:
        pip_output = env.RunCommand(*pip_cmd("list", "--format=json"))
        pip_data = json.loads(pip_output)
        packages = {p["name"]: pepver_to_semver(p["version"]) for p in pip_data}
    except:
        print(
            "LibreTiny: Warning! Couldn't extract the list of installed Python packages",
            file=sys.stderr,
        )
        packages = {}

    to_install = []
    for name, spec in dependencies.items():
        install_spec = f"{name}{dependencies[name]}"
        if name not in packages:
            to_install.append(install_spec)
        elif spec:
            version_spec = semantic_version.Spec(spec)
            if not version_spec.match(packages[name]):
                to_install.append(install_spec)

    if not to_install:
        return

    try:
        env.RunCommand(
            *pip_cmd("install", "-U", *to_install),
            message="LibreTiny: Installing Python dependencies",
        )
    except Exception as e:
        env.PrintCommandError(
            e,
            message="LibreTiny: Error! Couldn't install Python dependencies, exiting",
        )
        raise SystemExit(1)


env.AddMethod(env_configure_python_venv, "ConfigurePythonVenv")
env.AddMethod(env_install_python_dependencies, "InstallPythonDependencies")

# Copyright (c) Kuba Szczodrzyński 2022-06-13.

from os.path import isfile

from platformio.platform.base import PlatformBase
from SCons.Script import DefaultEnvironment, Environment

env: Environment = DefaultEnvironment()


def env_load_defines(env: Environment, path: str):
    path = env.subst(path)
    if not isfile(path):
        raise FileNotFoundError(f"Defines file not found ({path})")
    config = {}
    f = open(path, "r", encoding="utf-8")
    for line in f:
        line: str
        if not line.startswith("#define"):
            continue
        line = line[7:].strip()
        line = line.split(None, 2)
        if len(line) == 1:
            key, value = line[0], 1
        elif len(line) == 2:
            key, value = line[0], line[1]
        else:
            raise ValueError(f"Unknown directive: {line}")
        for tpl in env["CPPDEFINES"]:
            if isinstance(tpl, tuple):
                k = tpl[0]
            else:
                k = tpl
            if k == key:
                env["CPPDEFINES"].remove(tpl)
                break
        env.Append(CPPDEFINES=[(key, value)])
        config[key] = value
    env.Append(
        CONFIG=config,
    )
    f.close()


def env_load_config(env: Environment, path: str):
    path = env.subst(path)
    if not isfile(path):
        raise FileNotFoundError(f"Config file not found ({path})")
    STRIP_CHARS = "\t "
    config = {}
    f = open(path, "r", encoding="utf-8")
    for line in f:
        line: str
        if not line.startswith("#define"):
            continue
        line = line[7:].strip(STRIP_CHARS)
        parts = line.split(None, 2)
        if len(parts) == 2:
            key, value = parts
            value = value.strip(STRIP_CHARS)
        else:
            key = parts[0]
            value = "1"
        if value.isnumeric():
            value = int(value, 0)
        elif value.startswith('"') and value.endswith('"'):
            value = value[1:-1]
        else:
            # store defines as bytes
            value = value.encode()
        config[key] = value
    env.Append(
        CONFIG=config,
    )
    f.close()


def env_get_config(env: Environment, key: str, header: str = None):
    if header:
        # read from custom options first, if 'header' specified
        header = header.replace(".", "#")
        platform: PlatformBase = env.PioPlatform()
        opts: dict = platform.custom_opts.get("options", {})
        custom_opts: dict = opts.get(header, {})
        if key in custom_opts:
            value = custom_opts[key]
            if isinstance(value, str) and value.isnumeric():
                value = int(value, 0)
            return value

    config: dict = env["CONFIG"]
    if not config:
        return None
    value = config.get(key, None)
    while isinstance(value, bytes):
        value = config.get(value.decode(), None)
    return value


env.AddMethod(env_load_defines, "LoadDefines")
env.AddMethod(env_load_config, "LoadConfig")
env.AddMethod(env_get_config, "Cfg")
env.AddMethod(env_get_config, "GetConfig")

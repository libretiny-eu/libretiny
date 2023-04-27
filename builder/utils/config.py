# Copyright (c) Kuba Szczodrzyński 2022-06-13.

from os.path import isfile

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
            env.Append(CPPDEFINES=[(line[0], 1)])
            config[line[0]] = 1
        elif len(line) == 2:
            env.Append(CPPDEFINES=[(line[0], line[1])])
            config[line[0]] = line[1]
        else:
            raise ValueError(f"Unknown directive: {line}")
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
        (key, value) = line.split(None, 2)
        value = value.strip(STRIP_CHARS)
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


def env_get_config(env: Environment, key: str):
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

# Copyright (c) Kuba Szczodrzyński 2026-10-18.

import subprocess
import sys
from subprocess import CalledProcessError

from SCons.Script import DefaultEnvironment, Environment

env: Environment = DefaultEnvironment()


def env_run_command(env: Environment, *args: list[str], message: str = None):
    if message:
        print(message, file=sys.stderr)
    return subprocess.check_output(args)


def env_print_command_error(env: Environment, e: Exception, message: str = None):
    if message:
        print(message, file=sys.stderr)
    if isinstance(e, CalledProcessError):
        print(f"Process exited with return code {e.returncode} - stderr:")
        sys.stderr.buffer.write(e.stderr)
        sys.stderr.buffer.flush()
    else:
        print(f"Exception: {e}")


env.AddMethod(env_run_command, "RunCommand")
env.AddMethod(env_print_command_error, "PrintCommandError")

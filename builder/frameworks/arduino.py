# Copyright (c) Kuba Szczodrzyński 2022-05-16.

from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()
board = env.BoardConfig()
platform = env.PioPlatform()

# support passing "arduino" as framework
frameworks = board.get("frameworks")
framework = next(fw for fw in frameworks if "arduino" in fw)
builder = platform.frameworks[framework]["script"]
builder = builder.rpartition("/")[2]
env.SConscript(builder, exports="env")

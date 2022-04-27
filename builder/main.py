from SCons.Script import Default, DefaultEnvironment

env = DefaultEnvironment()
board = env.BoardConfig()

# Firmware name
if env.get("PROGNAME", "program") == "program":
    env.Replace(PROGNAME="firmware")
env.Replace(PROGSUFFIX=".elf")

# Toolchain config - TODO multiple arch, specified in board.json
env.Replace(
    AR="arm-none-eabi-gcc-ar",
    AS="arm-none-eabi-gcc",
    CC="arm-none-eabi-gcc",
    CXX="arm-none-eabi-g++",
    GDB="arm-none-eabi-gdb",
    NM="arm-none-eabi-gcc-nm",
    LINK="arm-none-eabi-gcc",
    LD="arm-none-eabi-gcc",
    OBJCOPY="arm-none-eabi-objcopy",
    OBJDUMP="arm-none-eabi-objdump",
    # RANLIB="arm-none-eabi-gcc-ranlib",
    SIZETOOL="arm-none-eabi-size",
)

# Flash layout defines
flash_layout: dict = board.get("flash")
if flash_layout:
    defines = {}
    for name, layout in flash_layout.items():
        name = name.upper()
        (offset, _, length) = layout.partition("+")
        defines[f"FLASH_{name}_OFFSET"] = offset
        defines[f"FLASH_{name}_LENGTH"] = length
    env.Append(CPPDEFINES=defines.items())
    env.Replace(**defines)

target_elf = env.BuildProgram()
target_fw = env.DumpFirmwareBinary("firmware.bin", target_elf)
Default(target_fw)

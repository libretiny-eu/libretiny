from SCons.Script import Default, DefaultEnvironment

env = DefaultEnvironment()

# Firmware name
if env.get("PROGNAME", "program") == "program":
    env.Replace(PROGNAME="firmware")
env.Replace(PROGSUFFIX=".elf")

env.Replace(
    AR="arm-none-eabi-gcc-ar",
    AS="arm-none-eabi-gcc",
    CC="arm-none-eabi-gcc",
    CXX="arm-none-eabi-g++",
    NM="arm-none-eabi-gcc-nm",
    LINK="arm-none-eabi-gcc",
    LD="arm-none-eabi-gcc",
    OBJCOPY="arm-none-eabi-objcopy",
    OBJDUMP="arm-none-eabi-objdump",
    # RANLIB="arm-none-eabi-gcc-ranlib",
    SIZETOOL="arm-none-eabi-size",
)

# I have no idea how does PlatformIO work
# env.SConscript("frameworks/realtek-ambz-sdk.py")

target_elf = env.BuildProgram()
target_fw = env.DumpFirmwareBinary("firmware.bin", target_elf)
Default(target_fw)

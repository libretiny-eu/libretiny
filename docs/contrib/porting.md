# Porting new families

This document briefly outlines what needs to be done, in order to port a new chip family to LibreTiny.

## Base framework + builders

The base framework is the core part, that provides little functionality and a small HAL (over some things like OTA or sys control). It also includes a builder script for the vendor SDK.

Here's what has to be done to make that work:

1. Find vendor SDK - should be self-explanatory. We can't work without a working SDK (yet).
2. Test vendor SDK - compile a sample program "as it was meant to be done".

	- Most SDKs provide some example programs (like Hello World, WiFi scanning, etc.) that can usually be compiled by running a single "make" command.
	- Sometimes you need to configure your environment in a weird and complicated way. For me, using Cygwin on Windows was usually enough, though.
	- You need to flash this to the chip as well. The SDK usually bundles some flashing tools.
	- This step is crucial to understand the vendor build system, and to have working binaries to compare out results against.

3. "Clean up" vendor SDK.

	- SDKs usually bundle entire compiler toolchains, which can take up hundreds of megabytes. We want to keep the downloaded PlatformIO packages as small as possible.
	- On existing families, GitHub Workflows produce the packages by removing some files and adding `package.json` to them. See [framework-beken-bdk/.github/workflows/platformio-package.yml](https://github.com/libretiny-eu/framework-beken-bdk/blob/actions/.github/workflows/platformio-package.yml) for an example.

4. Write base family and board definitions.

	- `families.json` needs to have the new family added to it.
	- `platform.json` needs to know the vendor SDK repository.
	- Add any boards and base JSONs to the `boards/` directory. It's easiest to start with generic boards.
	- Use `boardgen ltci` to generate variant sources (.c and .h).

5. Add base core code.

	- `lt_defs.h`, `lt_family.h` and `lt_api.c` files need to be created, and initialized with (even empty) functions and definitions.
	- The list of family functions can be found [here](lt-api.md).
	- Make the SDK call `lt_main()` as the entrypoint. If needed, use fixups.

6. Write a binary manipulation tool.

	- While this step could be optional, as these tools are provided in the SDK, they're usually platform-specific (i.e. Windows-only) and use proprietary executables, with no source code nor documentation. This is unacceptable for LibreTiny, as we need to support multiple architectures & platforms (Windows, Linux, Raspberry Pi, etc.). Naturally, doing that in Python seems to be the best choice.
	- All binary tools are currently in [ltchiptool/soc/.../binary.py](https://github.com/libretiny-eu/ltchiptool/blob/master/ltchiptool/soc/bk72xx/binary.py). The `elf2bin()` function is what takes an .ELF file, and generates a set of binaries that can be flashed to the chip.
	- It's best to test if the generation is correct, by taking an .ELF compiled by vendor SDK, running it through ltchiptool and checking if the resulting binaries are identical.
	- Ghidra/IDA Pro is your friend here; you can decompile the SDK tools.

7. Write a flashing tool.

	- mostly the same as above. Refer to the existing tools for examples. It's useful to make the flasher class "standalone", i.e. a class that is then wrapped by ltchiptool, like in [`realtek-ambz2`](https://github.com/libretiny-eu/ltchiptool/blob/master/ltchiptool/soc/ambz2/util/ambz2tool.py).

8. Write builder scripts.

	- `builder/family/xxx.py` files are builders, which contain all SDK sources and include paths. Write the script, based on the existing families, and any Makefiles or other scripts from the SDK.
	- Make sure not to make a mess in the `CCFLAGS`/`CPPDEFINES`, and only include what's needed there. Some flags are project-wide (family-independent) in `builder/frameworks/base.py`.
	- Use a **pure PlatformIO** project - **not ESPHome!**. Pass one of the generic boards you created before, and `framework = base` in `platformio.ini`. Generally, try to get the thing to compile.
	- Use a simple Hello World program - C, not C++. Only add `main()` function with a `printf()` and a `while(1)` loop.
	- I've noticed that using `nano.specs` instead of `nosys.specs` produces smaller binaries.

9. When you get it to link successfully, build a UF2 file.

	- UF2 packages are for flashing and for OTA.
	- Add `UF2OTA` to the env, to provide binaries that will go to the UF2. Some understanding of the chip's partition and flash layout will be needed.

10. Flash it, test if it works!

	- It probably won't. You may need to remove `__libc_init_array()` from `cores/common/base/lt_api.c` so that it doesn't crash. Most SDKs don't support C++ properly.

## Making it *actually* work

1. Write `flashdb` and `printf` ports.

	- The ports are in `cores/.../base/port/`. It's a simple flash access layer, and a character printing function. Not a lot of work, but it needs to be done first.

2. Add fixups so that string & memory stdlib functions are not from SDK.

	- Refer to [stdlib.md](stdlib.md) to find functions that need to be wrapped.
	- SDK should not define them, you have to figure out a way to remove them from headers. Fixups can mess with includes and trick the SDK into using our own functions.

3. Clean up FreeRTOS.

	- FreeRTOS' headers usually include some SDK headers, which pull in a lot of macros and typedefs, which usually break lots of non-SDK code, which doesn't expect these macros.
	- [library-freertos](https://github.com/libretiny-eu/library-freertos) repo contains some FreeRTOS versions, adapted for SDKs. Basically, copy a clean (straight from FreeRTOS github) version to the repo, commit it. Then copy the version from SDK and compare the differences.
	- Try to make it look as "stock" as possible. Discard any formatting differences (and backports).
	- Annotate any parts that can't be removed with `#ifdef FREERTOS_PORT_REALTEK_AMB1`.
	- Put the FreeRTOS vendor-specific port in [library-freertos-port](https://github.com/libretiny-eu/library-freertos-port).
	- Remove all FreeRTOS sources from builder scripts. Replace with:

	```py
	env.Replace(FREERTOS_PORT=env["FAMILY_NAME"], FREERTOS_PORT_DEFINE="REALTEK_AMB1")
	queue.AddExternalLibrary("freertos")
	queue.AddExternalLibrary("freertos-port")
	```

4. Do the same with lwIP - later.

5. Write LibreTiny C APIs - in `lt_api.c`.

6. At this point, your Hello World code should work fine.

## Porting Arduino Core - C++ support

1. Add main.cpp and write wiring_*.c ports. GPIOs and stuff should work even without proper C++ support.

2. Port Serial library first. This should already show whether C++ works fine or if it doesn't. For example, calling `Serial.println()` refers to the virtual function `Print::write`, which will probably crash the chip if C++ is not being linked properly.

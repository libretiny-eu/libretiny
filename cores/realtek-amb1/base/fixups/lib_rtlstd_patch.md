# lib_rtlstd_patch.a

```
arm-none-eabi-gcc-ar xo lib_rtlstd.a
rm rtl_eabi_cast_ram.o
arm-none-eabi-objcopy --strip-debug --strip-unneeded ram_libc.o
arm-none-eabi-objcopy --strip-debug --strip-unneeded ram_libgloss_retarget.o
arm-none-eabi-objcopy --strip-debug --strip-unneeded rtl_math_ram.o
arm-none-eabi-gcc-ar qs lib_rtlstd_patch.a *.o
```

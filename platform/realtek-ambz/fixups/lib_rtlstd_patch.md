# lib_rtlstd_patch.a

```
arm-none-eabi-gcc-ar xo lib_rtlstd.a
arm-none-eabi-objcopy -N __aeabi_fsub rtl_eabi_cast_ram.o
arm-none-eabi-objcopy -N __aeabi_fadd rtl_eabi_cast_ram.o
arm-none-eabi-objcopy -N __aeabi_ui2f rtl_eabi_cast_ram.o
arm-none-eabi-objcopy -N __aeabi_i2f rtl_eabi_cast_ram.o
arm-none-eabi-objcopy --strip-debug --strip-unneeded ram_libc.o
arm-none-eabi-objcopy --strip-debug --strip-unneeded ram_libgloss_retarget.o
arm-none-eabi-objcopy --strip-debug --strip-unneeded rtl_eabi_cast_ram.o
arm-none-eabi-objcopy --strip-debug --strip-unneeded rtl_math_ram.o
arm-none-eabi-gcc-ar qs lib_rtlstd_patch.a *.o
```


#ifdef RBCX_SBOOT_BUILTIN

#if RBCX_HW_VER == 0x0101
    .section .bootloader
    // https://github.com/RoboticsBrno/sboot_stm32/commit/c9220bc542a3fb87611215862110e5b2b703a9cd
    .incbin "src/bootloader/sboot_v11.bin"
#else
    #error "This RBCX_HW_VER does not have built-in sboot distribution implemented."
#endif

#endif // RBCX_SBOOT_BUILTIN

# Memory management

Function      | Target                 | #define location                                  | Notes
--------------|------------------------|---------------------------------------------------|------------------------------------------------------------------------------------------------
__`malloc`__  | __`pvPortMalloc`__     | `component/common/api/platform/platform_stdlib.h` |
`zalloc`      | `os_zalloc` (ROM)      |                                                   | This is **PROBABLY BROKEN**. ROM disassembly shows it only does memset on a fixed memory range.
__`zalloc`__  | __`pvPortZalloc`__     | `arduino/realtek-ambz/cores/WVariant.h`           | Custom implementation in `rtl_sys.cpp`
`calloc`      | `os_calloc`            | ?                                                 | This one is not in ROM. I didn't dig any deeper into it.
`calloc`      | `calloc_freertos`      | `component/os/freertos/cmsis_os.h`                | Probably not used
`calloc`      | `__rtl_calloc_r` (ROM) |                                                   | Not used, as I preferred to use FreeRTOS memory management.
__`calloc`__  | __`pvPortCalloc`__     | `arduino/realtek-ambz/cores/WVariant.h`           | Custom implementation in `rtl_sys.cpp`
__`realloc`__ | __`pvPortRealloc`__    | `arduino/realtek-ambz/cores/WVariant.h`           |
__`free`__    | __`vPortFree`__        | `component/common/api/platform/platform_stdlib.h` |

__Underlined__ item means that it is defined and used in code.

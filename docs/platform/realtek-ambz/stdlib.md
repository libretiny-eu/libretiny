# C library

The following is an auto-generated list of C standard library definitions included in the SDK. `Location` column contains the location of a matching symbol.

## ctype.h

### Character classification functions

Definition | Value      | Location
-----------|------------|---------
`isalnum`  |            |
`isalpha`  |            |
`isblank`  |            |
`iscntrl`  |            |
`isdigit`  | `in_range` |
`isgraph`  |            |
`islower`  | `in_range` |
`isprint`  | `in_range` |
`ispunct`  |            |
`isspace`  |            |
`isupper`  |            |
`isxdigit` |            |

## stdarg.h

Definition | Value                | Location
-----------|----------------------|---------
`va_start` | `__builtin_va_start` |
`va_arg`   | `__builtin_va_arg`   |
`va_end`   | `__builtin_va_end`   |
`va_copy`  | `__builtin_va_copy`  |

## stddef.h

Definition    | Value               | Location
--------------|---------------------|---------
`ptrdiff_t`   | `long int`          |
`size_t`      | `long unsigned int` |
`max_align_t` |                     |
`nullptr_t`   |                     |
`NULL`        |                     |

## stdint.h

### Types

Definition       | Value                | Location
-----------------|----------------------|---------
`intmax_t`       | `long int`           |
`uintmax_t`      | `long unsigned int`  |
`int8_t`         | `signed char`        |
`uint8_t`        | `unsigned char`      |
`int16_t`        | `short int`          |
`uint16_t`       | `short unsigned int` |
`int32_t`        | `int`                |
`uint32_t`       | `unsigned int`       |
`int64_t`        | `long int`           |
`uint64_t`       | `long unsigned int`  |
`int_least8_t`   | `signed char`        |
`uint_least8_t`  | `unsigned char`      |
`int_least16_t`  | `short int`          |
`uint_least16_t` | `short unsigned int` |
`int_least32_t`  | `int`                |
`uint_least32_t` | `unsigned int`       |
`int_least64_t`  | `long int`           |
`uint_least64_t` | `long unsigned int`  |
`int_fast8_t`    | `signed char`        |
`uint_fast8_t`   | `unsigned char`      |
`int_fast16_t`   | `long int`           |
`uint_fast16_t`  | `long unsigned int`  |
`int_fast32_t`   | `long int`           |
`uint_fast32_t`  | `long unsigned int`  |
`int_fast64_t`   | `long int`           |
`uint_fast64_t`  | `long unsigned int`  |
`intptr_t`       | `long int`           |
`uintptr_t`      | `long unsigned int`  |

## stdio.h

### Formatted input/output

Definition  | Value           | Location
------------|-----------------|-----------------
`fprintf`   |                 |
`fscanf`    |                 |
`printf`    | `rtl_printf`    | `lib_rtlstd.a`
`scanf`     |                 |
`snprintf`  | `rtl_snprintf`  | `lib_rtlstd.a`
`sprintf`   | `rtl_sprintf`   | `lib_rtlstd.a`
`sscanf`    | `_sscanf_patch` | `lib_platform.a`
`vfprintf`  |                 |
`vfscanf`   |                 |
`vprintf`   |                 |
`vscanf`    |                 |
`vsnprintf` | `rtl_vsnprintf` | `lib_rtlstd.a`
`vsprintf`  |                 |
`vsscanf`   |                 |

### Error-handling

Definition | Value         | Location
-----------|---------------|-------------
`clearerr` | `__sclearerr` |
`feof`     | `__sfeof`     |
`ferror`   | `__sferror`   |
`perror`   |               | `lib_mdns.a`

## stdlib.h

### String conversion

Definition | Value            | Location
-----------|------------------|---------
`atof`     |                  |
`atoi`     | `prvAtoi`        | ROM
`atol`     | `simple_strtol`  | ROM
`atoll`    |                  |
`strtod`   |                  |
`strtof`   |                  |
`strtol`   | `simple_strtol`  | ROM
`strtold`  |                  |
`strtoll`  |                  |
`strtoul`  | `simple_strtoul` | ROM
`strtoull` |                  |

### Pseudo-random sequence generation

Definition | Value  | Location
-----------|--------|---------
`rand`     | `Rand` | ROM
`srand`    |        |

### Dynamic memory management

Definition | Value          | Location
-----------|----------------|---------
`calloc`   |                |
`free`     | `vPortFree`    | SDK
`malloc`   | `pvPortMalloc` | SDK
`realloc`  |                |

## string.h

### Copying

Definition | Value                 | Location
-----------|-----------------------|---------
`memcpy`   | `_memcpy`             | ROM
`memmove`  | `__rtl_memmove_v1_00` | ROM
`strcpy`   | `_strcpy`             | ROM
`strncpy`  | `_strncpy`            | ROM

### Concatenation

Definition | Value                 | Location
-----------|-----------------------|---------
`strcat`   | `__rtl_strcat_v1_00`  | ROM
`strncat`  | `__rtl_strncat_v1_00` | ROM

### Comparison

Definition | Value       | Location
-----------|-------------|---------
`memcmp`   | `_memcmp`   | ROM
`strcmp`   | `prvStrCmp` | ROM
`strcoll`  |             |
`strncmp`  | `_strncmp`  | ROM
`strxfrm`  |             |

### Searching

Definition | Value                | Location
-----------|----------------------|---------
`memchr`   | `__rtl_memchr_v1_00` | ROM
`strchr`   | `_strchr`            | ROM
`strcspn`  |                      |
`strpbrk`  | `_strpbrk`           | ROM
`strrchr`  |                      |
`strspn`   |                      |
`strstr`   | `prvStrStr`          | ROM
`strtok`   | `prvStrtok`          | ROM

### Other

Definition | Value       | Location
-----------|-------------|---------
`memset`   | `_memset`   | ROM
`strerror` |             |
`strlen`   | `prvStrLen` | ROM

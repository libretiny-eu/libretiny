/* Copyright (c) Kuba Szczodrzyński 2022-04-23. */

/* WCharacter.h uses deprecated isascii() and toascii(),
   which are available in gnu++11 but not c++11 */
#include <cstddef>

#ifdef __STRICT_ANSI__

inline bool isascii(int c) {
  return ((c & ~0x7f) != 0 ? false : true);
}

inline bool toascii(int c) {
  return (c & 0x7f);
}

#endif

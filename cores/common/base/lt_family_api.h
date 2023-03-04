/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#pragma once

#include <libretuya.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Get the reason of last chip reset.
 */
ResetReason lt_get_reset_reason();

#ifdef __cplusplus
} // extern "C"
#endif

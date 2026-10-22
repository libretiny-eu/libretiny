/* Copyright (c) Kuba Szczodrzyński 2026-10-20. */

#pragma once

#include <libretiny.h>

/**
 * @brief Get LibreTiny version string.
 */
const char *lt_get_version();

/**
 * @brief Get the compilation timestamp as string (usually "MMM DD YYYY HH:MM:SS").
 * This can be overridden by defining LT_BUILD_TIMESTAMP, e.g. for reproducible builds.
 */
const char *lt_get_build_timestamp();

/**
 * @brief Get board code.
 */
const char *lt_get_board_code();

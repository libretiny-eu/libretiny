/* Copyright (c) Bl00d-B0b 2026-08-07. */

#pragma once

// lwIP's port typedefs u32_t as unsigned int, so its socklen_t differs from the
// uint32_t (unsigned long here) ESPHome's socket layer uses on Arduino targets.
// Define socklen_t once and tell lwIP to skip its own.

#include_next <arch/cc.h>

#include <stdint.h>

#ifndef SOCKLEN_T_DEFINED
#define SOCKLEN_T_DEFINED
typedef uint32_t socklen_t;
#endif

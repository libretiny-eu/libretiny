/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#include <libretiny.h>
#include <sdk_private.h>

uint32_t lt_ram_get_size() {
	return (256 + 2048) * 1024;
}

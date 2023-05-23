/* Copyright (c) Kuba Szczodrzyński 2023-05-23. */

#include <libretiny.h>
#include <sdk_private.h>

/*______ _           _
 |  ____| |         | |
 | |__  | | __ _ ___| |__
 |  __| | |/ _` / __| '_ \
 | |    | | (_| \__ \ | | |
 |_|    |_|\__,_|___/_| |*/
lt_flash_id_t lt_flash_get_id() {
	lt_flash_id_t id;
	uint8_t idBytes[3];
	flash_read_id(&lt_flash_obj, idBytes, 3);
	id.manufacturer_id = idBytes[0];
	id.chip_id		   = idBytes[1];
	id.chip_size_id	   = idBytes[2];
	return id;
}

/*_          __   _       _         _
 \ \        / /  | |     | |       | |
  \ \  /\  / /_ _| |_ ___| |__   __| | ___   __ _
   \ \/  \/ / _` | __/ __| '_ \ / _` |/ _ \ / _` |
	\  /\  / (_| | || (__| | | | (_| | (_) | (_| |
	 \/  \/ \__,_|\__\___|_| |_|\__,_|\___/ \__, |
											 __/ |
											|___*/
bool lt_wdt_enable(uint32_t timeout) {
	watchdog_init(timeout);
	watchdog_start();
	return true;
}

void lt_wdt_disable() {
	watchdog_stop();
}

void lt_wdt_feed() {
	watchdog_refresh();
}

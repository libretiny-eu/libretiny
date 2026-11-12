/* Requires --wrap for every symbol listed in PORTING.md. No success stubs. */
#include <libretiny.h>
#include <sdk_private.h>

static void denied(const char *operation) {
	LT_E("AMB1 read-only flash: denied SDK %s", operation);
}

int __wrap_flash_write_word(flash_t *obj, uint32_t address, uint32_t data) {
	(void)obj;
	(void)address;
	(void)data;
	denied("flash_write_word");
	return 0; /* SDK contract: 1 = success, all other values = failure. */
}

int __wrap_flash_stream_write(flash_t *obj, uint32_t address, uint32_t length, uint8_t *data) {
	(void)obj;
	(void)address;
	(void)length;
	(void)data;
	denied("flash_stream_write");
	return 0;
}

int __wrap_flash_burst_write(flash_t *obj, uint32_t address, uint32_t length, uint8_t *data) {
	(void)obj;
	(void)address;
	(void)length;
	(void)data;
	denied("flash_burst_write");
	return 0;
}

void __wrap_flash_erase_sector(flash_t *obj, uint32_t address) {
	(void)obj;
	(void)address;
	denied("flash_erase_sector (void API cannot report failure)");
}

void __wrap_flash_erase_block(flash_t *obj, uint32_t address) {
	(void)obj;
	(void)address;
	denied("flash_erase_block (void API cannot report failure)");
}

void __wrap_flash_erase_chip(flash_t *obj) {
	(void)obj;
	denied("flash_erase_chip (void API cannot report failure)");
}

void __wrap_flash_write_protect(flash_t *obj, uint32_t protect) {
	(void)obj;
	(void)protect;
	denied("flash_write_protect (void API cannot report failure)");
}

int __wrap_flash_set_status(flash_t *obj, uint32_t data) {
	(void)obj;
	(void)data;
	denied("flash_set_status");
	return 0;
}

void __wrap_flash_reset_status(flash_t *obj) {
	(void)obj;
	denied("flash_reset_status (void API cannot report failure)");
}

int __wrap_flash_set_extend_addr(flash_t *obj, uint32_t data) {
	(void)obj;
	(void)data;
	denied("flash_set_extend_addr");
	return 0;
}

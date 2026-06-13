#pragma once
#include <stdbool.h>
#include <stdint.h>

#define LT_OTA_META_MAGIC	  0x4F544131u // "OTA1"
#define LT_OTA_BANK_COUNT	  2
#define LT_OTA_TRIAL_ATTEMPTS 3 // boots a trial bank gets before auto-revert

// Per-bank descriptor. CRC32 is computed over [bank_base .. bank_base+length).
typedef struct {
	uint32_t length; // image length in bytes; 0 = empty
	uint32_t crc32;	 // CRC32 over the image body
	uint8_t state;	 // lt_ota_bank_state_t
	uint8_t pad[3];
} lt_ota_bank_desc_t;

typedef enum {
	LT_OTA_BANK_EMPTY	  = 0, // no image
	LT_OTA_BANK_VALID	  = 1, // committed, known-good
	LT_OTA_BANK_TRIAL	  = 2, // newly written, on trial
	LT_OTA_BANK_CONFIRMED = 3, // confirmed healthy (treated as VALID by the loader)
} lt_ota_bank_state_t;

typedef struct {
	uint32_t magic;	   // LT_OTA_META_MAGIC
	uint32_t seq;	   // higher valid seq wins; wraps handled in code
	uint8_t boot_bank; // 0 = bank A, 1 = bank B
	uint8_t attempts;  // trial attempts remaining for boot_bank
	uint8_t pad[2];
	lt_ota_bank_desc_t bank[LT_OTA_BANK_COUNT];
	uint32_t crc32; // CRC32 over all preceding bytes of this struct
} lt_ota_meta_t;

// Flash access injected by the caller (app uses FAL; bootloader uses bare MSC).
typedef struct {
	int (*read)(uint32_t off, void *buf, uint32_t len);		   // 0 ok, <0 err
	int (*erase)(uint32_t off, uint32_t len);				   // page-aligned
	int (*write)(uint32_t off, const void *buf, uint32_t len); // word-aligned
} lt_ota_flash_ops_t;

// Layout constants (absolute flash offsets). Mirror the board JSON flash map.
#define LT_OTA_META_SLOT0_OFF 0x1F8000u
#define LT_OTA_META_SLOT1_OFF 0x1F9000u
#define LT_OTA_META_SLOT_LEN  0x1000u // one 4 KB page per slot
#define LT_OTA_BANK_A_OFF	  0x008000u
#define LT_OTA_BANK_B_OFF	  0x100000u
// 0x0F0000 (960 KB), not the 0x0F8000 the bank spacing would allow: the last
// 32 KB before metadata is carved out for the `kvs` FlashDB partition
// (0x1F0000+0x8000), which lives outside both banks so it survives OTA. Bank A
// is shrunk to match (its freed tail 0x0F8000..0x100000 is left unused) so the
// same-sized image links into either bank. Mirrors the board JSON flash map.
#define LT_OTA_BANK_LEN		  0x0F0000u

uint32_t lt_ota_bank_offset(uint8_t bank);								  // 0->A,1->B
bool lt_ota_meta_load(const lt_ota_flash_ops_t *ops, lt_ota_meta_t *out); // newest valid slot
bool lt_ota_meta_store(const lt_ota_flash_ops_t *ops, lt_ota_meta_t *m);  // ping-pong write (bumps seq)
bool lt_ota_meta_bank_valid(const lt_ota_flash_ops_t *ops, const lt_ota_meta_t *m, uint8_t bank);

# Host unit tests for the portable OTA logic (CRC32 + metadata ping-pong).
# Pure C, no hardware. Run: make -C cores/silabs-efm32gg11/test/ota
CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -Werror -g -I../../base/ota
TESTS    = test_crc32 test_meta

.PHONY: all clean
all: $(TESTS)
	@for t in $(TESTS); do echo "== $$t =="; ./$$t || exit 1; done

test_crc32: test_crc32.c ../../base/ota/lt_crc32.c
	$(CC) $(CFLAGS) -o $@ $^

test_meta: test_meta.c ../../base/ota/lt_meta_hostshim.c ../../base/ota/lt_ota_meta.c ../../base/ota/lt_crc32.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TESTS)

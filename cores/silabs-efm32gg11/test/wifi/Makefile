# Host unit tests for the portable WiFi logic (STA auto-reconnect decision).
# Pure C, no hardware. Run: make -C cores/silabs-efm32gg11/test/wifi
CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -Werror -g -I../../base/wifi
TESTS    = test_reconnect

.PHONY: all clean
all: $(TESTS)
	@for t in $(TESTS); do echo "== $$t =="; ./$$t && echo "PASS" || exit 1; done

test_reconnect: test_reconnect.c ../../base/wifi/lt_wifi_reconnect.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TESTS)

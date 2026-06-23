# Host unit tests for the portable PWM math. Pure C, no hardware.
# Run: make -C cores/silabs-efm32gg11/test/pwm
CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -Werror -g -I../../arduino/src
TESTS    = test_pwm

.PHONY: all clean
all: $(TESTS)
	@for t in $(TESTS); do echo "== $$t =="; ./$$t && echo "PASS" || exit 1; done

test_pwm: test_pwm.c ../../arduino/src/lt_pwm.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TESTS)

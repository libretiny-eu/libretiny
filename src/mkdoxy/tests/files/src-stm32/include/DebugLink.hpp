#pragma once

#include <stdint.h>
#include <sys/types.h>

void debugUartInit();
void debugLinkPoll();
ssize_t debugLinkTx(const uint8_t* data, size_t len);

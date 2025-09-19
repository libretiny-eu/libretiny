#pragma once

#include <stddef.h>
#include <stdint.h>

#include "rbcx.pb.h"

void controlUartInit();
void controlLinkTx(const CoprocStat& outgoing);
bool controlLinkRx(CoprocReq& incoming);
void controlLinkReset();

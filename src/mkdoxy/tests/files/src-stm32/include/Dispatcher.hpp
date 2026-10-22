#pragma once

#include "rbcx.pb.h"

void dispatcherInit();
bool dispatcherEnqueueStatus(const CoprocStat& status);
bool dispatcherEnqueueRequest(const CoprocReq& request);
void dispatcherPoll();
void dispatcherReset();

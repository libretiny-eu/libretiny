#pragma once

#include "rbcx.pb.h"

void stupidServoInit();
void stupidServoDispatch(const CoprocReq_SetStupidServo& request);
void stupidServoReset();

#pragma once

#include "rbcx.pb.h"

void motorInit();
void motorDispatch(const CoprocReq_MotorReq& request);
void motorReset();

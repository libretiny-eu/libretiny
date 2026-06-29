#pragma once

#include "rbcx.pb.h"

void ultrasoundInit();
void ultrasoundDispatch(const CoprocReq_UltrasoundReq& request);
void ultrasoundOnEchoEdge();
void ultrasoundReset();

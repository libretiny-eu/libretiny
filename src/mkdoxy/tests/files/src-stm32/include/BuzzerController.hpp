#pragma once

#include "Bsp.hpp"

inline void buzzerSetState(bool on) { pinWrite(buzzerPin, on); }

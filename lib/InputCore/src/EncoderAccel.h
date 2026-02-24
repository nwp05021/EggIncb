#pragma once
#include <stdint.h>
#include "EncoderEvents.h"
#include "EncoderConfig.h"

class EncoderAccel {
public:
  void begin(const EncoderAccelConfig& cfg);
  EncoderEvents apply(const EncoderEvents& in, uint32_t nowMs);

private:
  EncoderAccelConfig cfg{};
  uint32_t lastStepMs = 0;
};

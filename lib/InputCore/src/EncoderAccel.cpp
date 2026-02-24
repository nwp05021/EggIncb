#include "EncoderAccel.h"

void EncoderAccel::begin(const EncoderAccelConfig& cfg_) {
  cfg = cfg_;
  lastStepMs = 0;
}

EncoderEvents EncoderAccel::apply(const EncoderEvents& in, uint32_t nowMs) {
  EncoderEvents out = in;
  if (in.delta == 0) return out;

  uint32_t dt = (lastStepMs == 0) ? 1000 : (nowMs - lastStepMs);
  lastStepMs = nowMs;

  int mul = 1;
  if (dt < cfg.dt8x) mul = cfg.mul8;
  else if (dt < cfg.dt4x) mul = cfg.mul4;
  else if (dt < cfg.dt2x) mul = cfg.mul2;

  out.delta = (int16_t)(in.delta * mul);
  return out;
}

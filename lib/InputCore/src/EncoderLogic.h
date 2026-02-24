#pragma once
#include <stdint.h>
#include "EncoderEvents.h"
#include "EncoderConfig.h"

class EncoderLogic {
public:
  void begin(const EncoderConfig& cfg, uint8_t initialAB, int initialBtnRaw);
  void onIsrAB(uint8_t currAB);
  int16_t takeIsrDeltaSnapshot();
  EncoderEvents poll(uint32_t nowMs, int btnRaw);

private:
  EncoderConfig cfg{};

  uint8_t prevAB = 0;
  int8_t  acc = 0;

  volatile int16_t isrDeltaAccum = 0;
  int16_t pendingDelta = 0;

  int btnStable = 1;
  int btnRawPrev = 1;
  uint32_t btnRawChangeMs = 0;

  bool pressed = false;
  uint32_t pressStartMs = 0;
  bool veryLongFired = false;
};

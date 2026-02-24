#pragma once
#include <stdint.h>
#include <Arduino.h> // ensures IRAM_ATTR is defined on ESP32 Arduino
#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

#include "EncoderHal.h"
#include "EncoderLogic.h"
#include "EncoderConfig.h"

class EncoderController {
public:
  explicit EncoderController(EncoderHal& hal);
  void begin(const EncoderConfig& cfg);

  EncoderEvents poll();

private:
  EncoderHal& hal;
  EncoderLogic logic;

  static EncoderController* instance;
  static void IRAM_ATTR isrRouter();
  void IRAM_ATTR handleIsr();
};

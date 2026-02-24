#pragma once
#include <stdint.h>

// GrowBed-like conservative defaults
struct EncoderConfig {
  uint32_t btnDebounceMs = 30;
  uint32_t longPressMs = 800;
  uint32_t veryLongPressMs = 5000; // engineering
};

// Acceleration curve for "hand feel"
struct EncoderAccelConfig {
  uint16_t dt2x = 120;
  uint16_t dt4x = 60;
  uint16_t dt8x = 30;

  uint8_t mul2 = 2;
  uint8_t mul4 = 4;
  uint8_t mul8 = 8;
};

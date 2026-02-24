#pragma once
#include <stdint.h>

struct EncoderEvents {
  int16_t delta = 0;        // one detent step per event (after accel applied at upper layer)
  bool shortPress = false;  // released before long threshold
  bool longPress = false;   // released after long threshold
  bool veryLongPress = false; // fired while holding (engineering)
};

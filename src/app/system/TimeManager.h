#pragma once
#include <stdint.h>

class TimeManager {
public:
  void begin(uint32_t bootMs, uint32_t lastKnownEpoch);
  void tick(uint32_t nowMs);

  bool hasTime() const { return _hasTime; }
  uint32_t nowEpoch() const { return _nowEpoch; } // seconds

  uint8_t incubationDay(uint32_t startEpoch) const;

  void setEpoch(uint32_t epochSec); // future NTP/RTC hook

private:
  bool _hasTime = false;
  uint32_t _lastTickMs = 0;
  uint32_t _nowEpoch = 0; // seconds
};

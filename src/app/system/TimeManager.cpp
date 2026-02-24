#include "TimeManager.h"

void TimeManager::begin(uint32_t bootMs, uint32_t lastKnownEpoch) {
  _lastTickMs = bootMs;
  if (lastKnownEpoch != 0) {
    _hasTime = true;
    _nowEpoch = lastKnownEpoch;
  }
}

void TimeManager::tick(uint32_t nowMs) {
  if (!_hasTime) return;

  uint32_t dtMs = nowMs - _lastTickMs;
  if (dtMs >= 1000) {
    uint32_t add = dtMs / 1000;
    _nowEpoch += add;
    _lastTickMs += add * 1000;
  }
}

void TimeManager::setEpoch(uint32_t epochSec) {
  _hasTime = true;
  _nowEpoch = epochSec;
}

uint8_t TimeManager::incubationDay(uint32_t startEpoch) const {
  if (!_hasTime || startEpoch == 0) return 1;
  uint32_t diff = (_nowEpoch > startEpoch) ? (_nowEpoch - startEpoch) : 0;
  uint32_t day = 1 + (diff / 86400UL);
  if (day > 99) day = 99;
  return (uint8_t)day;
}

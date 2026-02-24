#pragma once
#include <stdint.h>

class MotorSchedule {
public:
  void configure(uint16_t onSec, uint16_t offMin);
  void update(uint32_t nowMs);
  bool motorOn() const { return _state; }

private:
  uint16_t _onSec = 10;
  uint16_t _offMin = 30;
  uint32_t _lastToggle = 0;
  bool _state = false;
};

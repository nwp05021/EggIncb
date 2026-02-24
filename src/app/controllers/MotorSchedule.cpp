#include "MotorSchedule.h"

void MotorSchedule::configure(uint16_t onSec, uint16_t offMin) {
  _onSec = onSec;
  _offMin = offMin;
}

void MotorSchedule::update(uint32_t nowMs) {
  if (_lastToggle == 0) {
    _lastToggle = nowMs;
    _state = false;
    return;
  }

  uint32_t elapsed = nowMs - _lastToggle;
  uint32_t limit = _state ? (uint32_t)_onSec * 1000UL
                          : (uint32_t)_offMin * 60UL * 1000UL;

  if (elapsed >= limit) {
    _state = !_state;
    _lastToggle = nowMs;
  }
}

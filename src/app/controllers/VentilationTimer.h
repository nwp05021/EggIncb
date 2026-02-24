#pragma once
#include <stdint.h>

class VentilationTimer {
public:
  void configure(uint32_t intervalMs, uint32_t durationMs) {
    _intervalMs = intervalMs;
    _durationMs = durationMs;
  }

  void update(uint32_t nowMs) {
    if (_lastStartMs == 0) {
      _lastStartMs = nowMs;
      _active = false;
      return;
    }

    if (_active) {
      if (nowMs - _lastStartMs >= _durationMs) {
        _active = false;
        // next cycle starts from the end of ventilation window
        _lastStartMs = nowMs;
      }
      return;
    }

    if (nowMs - _lastStartMs >= _intervalMs) {
      _active = true;
      _lastStartMs = nowMs;
    }
  }

  bool active() const { return _active; }

private:
  uint32_t _intervalMs = 30UL * 60UL * 1000UL; // 30 min
  uint32_t _durationMs = 2UL * 60UL * 1000UL;  // 2 min
  uint32_t _lastStartMs = 0;
  bool _active = false;
};

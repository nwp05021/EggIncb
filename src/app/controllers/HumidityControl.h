#pragma once
#include <stdint.h>

class HumidityControl {
public:
  void configure(int16_t target_x10, int16_t hyst_x10) {
    _target = target_x10;
    _hyst = hyst_x10;
  }

  void update(int16_t current_x10) {
    if (current_x10 <= (_target - _hyst)) _on = true;
    if (current_x10 >= (_target + _hyst)) _on = false;
  }

  bool humidifierOn() const { return _on; }

private:
  int16_t _target = 0;
  int16_t _hyst = 0;
  bool _on = false;
};

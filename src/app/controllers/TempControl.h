#pragma once
#include <stdint.h>

class TempControl {
public:
  void configure(int16_t target_x10, int16_t hyst_x10);
  void update(int16_t current_x10);
  bool heaterOn() const { return _heater; }

private:
  int16_t _target = 0;
  int16_t _hyst = 0;
  bool _heater = false;
};

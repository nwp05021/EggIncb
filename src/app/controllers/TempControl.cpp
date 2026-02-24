#include "TempControl.h"

void TempControl::configure(int16_t target_x10, int16_t hyst_x10) {
  _target = target_x10;
  _hyst = hyst_x10;
}

void TempControl::update(int16_t current_x10) {
  if (current_x10 <= (_target - _hyst)) _heater = true;
  if (current_x10 >= (_target + _hyst)) _heater = false;
}

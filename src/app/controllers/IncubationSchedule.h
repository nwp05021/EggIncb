#pragma once
#include <stdint.h>

struct IncubationTargets {
  int16_t temp_x10;
  int16_t hum_x10;
};

struct IncubationStage {
  uint8_t startDay;
  uint8_t endDay;
  IncubationTargets t;
};

class IncubationSchedule {
public:
  // Default chicken profile: D1~18, D19~21
  static IncubationTargets targetsForDay(uint8_t day) {
    if (day < 1) day = 1;
    if (day > 21) day = 21;

    static const IncubationStage stages[] = {
      { 1, 18, { 375, 550 } }, // 37.5C, 55%
      {19, 21, { 372, 700 } }, // 37.2C, 70%
    };

    for (const auto& s : stages) {
      if (day >= s.startDay && day <= s.endDay) return s.t;
    }
    return {375, 550};
  }
};

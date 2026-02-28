#pragma once

#include <stdint.h>

// Shared clipboard for schedule table copy/paste operations.
// Stored in RAM only (not persisted).

struct ScheduleClipboard {
  bool    valid = false;
  uint8_t len   = 0;      // number of days copied
  int16_t temp_x10[21] = {0};
  int16_t hum_x10[21]  = {0};
};

// Accessor for the single clipboard instance.
ScheduleClipboard& scheduleClipboard();

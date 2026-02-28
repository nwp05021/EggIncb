#include "ScheduleClipboard.h"

static ScheduleClipboard g_clip;

ScheduleClipboard& scheduleClipboard() {
  return g_clip;
}

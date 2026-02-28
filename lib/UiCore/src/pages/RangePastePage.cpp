#include "RangePastePage.h"

#include <Arduino.h>

#include "../UiRenderer.h"
#include "../PageManager.h"
#include "../UiApp.h"
#include "../UiModel.h"

#include "ScheduleClipboard.h"

static int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

RangePastePage::RangePastePage(UiModel& m, PageManager& mgr, UiApp* app)
  : _m(m), _mgr(mgr), _app(app) {}

void RangePastePage::bindTable(int16_t* dayTempTable_x10, int16_t* dayHumTable_x10) {
  _pTemp = dayTempTable_x10;
  _pHum  = dayHumTable_x10;
}

void RangePastePage::onEnter() {
  _cursor = 0;
  _editing = false;
  _destDay = 1;
}

void RangePastePage::onEncoder(int delta) {
  if (delta == 0) return;
  int dir = (delta > 0) ? 1 : -1;

  if (_editing) {
    if ((Item)_cursor == Item::Dest) {
      _destDay = (uint8_t)clampi((int)_destDay + dir, 1, 21);
    }
    return;
  }

  int next = (int)_cursor + dir;
  if (next < 0) next = 0;
  if (next >= (int)Item::COUNT) next = (int)Item::COUNT - 1;
  _cursor = (uint8_t)next;
}

void RangePastePage::doPaste() {
  if (!_pTemp || !_pHum || !_app) return;
  ScheduleClipboard& clip = scheduleClipboard();
  if (!clip.valid || clip.len == 0) return;

  int dest = (int)_destDay - 1;
  int len = (int)clip.len;
  if (dest < 0) dest = 0;
  if (dest > 20) dest = 20;
  if (dest + len > 21) len = 21 - dest;
  if (len <= 0) return;

  for (int i = 0; i < len; ++i) {
    _pTemp[dest + i] = clip.temp_x10[i];
    _pHum[dest + i]  = clip.hum_x10[i];
  }

  _app->notifyConfigChanged();
}

void RangePastePage::onClick() {
  if (_editing) {
    _editing = false;
    return;
  }

  switch ((Item)_cursor) {
    case Item::Dest:
      _editing = true;
      break;
    case Item::Paste:
      doPaste();
      _mgr.pop();
      break;
    case Item::Back:
    default:
      _mgr.pop();
      break;
  }
}

void RangePastePage::onLongPress() {
  _mgr.pop();
}

void RangePastePage::render(UiRenderer& r) {
  static const char* labels[(int)Item::COUNT] = { "DEST", "PASTE", "BACK" };

  char values[(int)Item::COUNT][16];
  for (int i = 0; i < (int)Item::COUNT; ++i) values[i][0] = 0;

  ScheduleClipboard& clip = scheduleClipboard();
  snprintf(values[(int)Item::Dest], 16, "D%02u", (unsigned)_destDay);
  if (clip.valid) snprintf(values[(int)Item::Paste], 16, "LEN%u", (unsigned)clip.len);
  else snprintf(values[(int)Item::Paste], 16, "(no)" );

  const char* vptrs[(int)Item::COUNT];
  for (int i = 0; i < (int)Item::COUNT; ++i) vptrs[i] = values[i];

  char titleBuf[20];
  snprintf(titleBuf, sizeof(titleBuf), "PASTE%s", _editing ? "*" : "");
  r.drawSettingsMenu(_m, titleBuf, labels, vptrs, (int)Item::COUNT, _cursor, 0, 4);
}

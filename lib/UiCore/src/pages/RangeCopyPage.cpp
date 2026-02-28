#include "RangeCopyPage.h"

#include <Arduino.h>

#include "../UiRenderer.h"
#include "../PageManager.h"
#include "../UiApp.h"
#include "../UiModel.h"

#include "ScheduleClipboard.h"

static int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

RangeCopyPage::RangeCopyPage(UiModel& m, PageManager& mgr, UiApp* app)
  : _m(m), _mgr(mgr), _app(app) {}

void RangeCopyPage::bindTable(int16_t* dayTempTable_x10, int16_t* dayHumTable_x10) {
  _pTemp = dayTempTable_x10;
  _pHum  = dayHumTable_x10;
}

const char* RangeCopyPage::presetName(RangePreset p) const {
  switch (p) {
    case RangePreset::Custom: return "CUSTOM";
    case RangePreset::D01_18: return "01-18";
    case RangePreset::D19_21: return "19-21";
    case RangePreset::D01_21: return "01-21";
    default: return "?";
  }
}

void RangeCopyPage::applyRangePreset(RangePreset p) {
  _preset = p;
  switch (p) {
    case RangePreset::D01_18: _startDay = 1;  _endDay = 18; break;
    case RangePreset::D19_21: _startDay = 19; _endDay = 21; break;
    case RangePreset::D01_21: _startDay = 1;  _endDay = 21; break;
    case RangePreset::Custom:
    default:
      break;
  }
  clampRange();
}

void RangeCopyPage::onEnter() {
  _cursor = 0;
  _page = 0;
  _editing = false;
  _preset = RangePreset::D01_21;
  _startDay = 1;
  _endDay = 21;
  clampRange();
}

void RangeCopyPage::clampRange() {
  _startDay = (uint8_t)clampi(_startDay, 1, 21);
  _endDay   = (uint8_t)clampi(_endDay,   1, 21);
  if (_startDay > _endDay) _endDay = _startDay;
}

void RangeCopyPage::onEncoder(int delta) {
  if (delta == 0) return;
  int dir = (delta > 0) ? 1 : -1;

  if (_editing) {
    if ((Item)_cursor == Item::Preset) {
      uint8_t n = (uint8_t)_preset;
      if (dir > 0) n = (uint8_t)((n + 1) % (uint8_t)RangePreset::COUNT);
      else n = (uint8_t)((n + (uint8_t)RangePreset::COUNT - 1) % (uint8_t)RangePreset::COUNT);
      applyRangePreset((RangePreset)n);
    } else if ((Item)_cursor == Item::Start) {
      _startDay = (uint8_t)clampi((int)_startDay + dir, 1, 21);
      _preset = RangePreset::Custom;
      if (_startDay > _endDay) _endDay = _startDay;
    } else if ((Item)_cursor == Item::End) {
      _endDay = (uint8_t)clampi((int)_endDay + dir, 1, 21);
      _preset = RangePreset::Custom;
      if (_endDay < _startDay) _startDay = _endDay;
    }
    return;
  }

  int next = (int)_cursor + dir;
  if (next < 0) next = 0;
  if (next >= (int)Item::COUNT) next = (int)Item::COUNT - 1;
  _cursor = (uint8_t)next;

  const int PAGE_SIZE = 4;
  int maxPage = ((int)Item::COUNT - 1) / PAGE_SIZE;
  _page = (uint8_t)clampi((int)(_cursor / PAGE_SIZE), 0, maxPage);
}

void RangeCopyPage::doCopy() {
  clampRange();
  if (!_pTemp || !_pHum) return;

  ScheduleClipboard& clip = scheduleClipboard();
  int s = (int)_startDay - 1;
  int e = (int)_endDay - 1;
  int len = e - s + 1;
  if (len <= 0) return;
  if (len > 21) len = 21;

  clip.valid = true;
  clip.len = (uint8_t)len;
  for (int i = 0; i < len; ++i) {
    clip.temp_x10[i] = _pTemp[s + i];
    clip.hum_x10[i]  = _pHum[s + i];
  }
}

void RangeCopyPage::onClick() {
  if (_editing) {
    _editing = false;
    return;
  }

  switch ((Item)_cursor) {
    case Item::Preset: {
      uint8_t n = (uint8_t)_preset + 1;
      if (n >= (uint8_t)RangePreset::COUNT) n = 0;
      applyRangePreset((RangePreset)n);
      break;
    }
    case Item::Start:
    case Item::End:
      _editing = true;
      break;
    case Item::Copy:
      doCopy();
      _mgr.pop();
      break;
    case Item::Back:
    default:
      _mgr.pop();
      break;
  }
}

void RangeCopyPage::onLongPress() {
  _mgr.pop();
}

void RangeCopyPage::render(UiRenderer& r) {
  static const char* labels[(int)Item::COUNT] = {
    "RNG", "START", "END", "COPY", "BACK"
  };

  char values[(int)Item::COUNT][16];
  for (int i = 0; i < (int)Item::COUNT; ++i) values[i][0] = 0;

  snprintf(values[(int)Item::Preset], 16, "%s", presetName(_preset));

  snprintf(values[(int)Item::Start], 16, "D%02u", (unsigned)_startDay);
  snprintf(values[(int)Item::End],   16, "D%02u", (unsigned)_endDay);

  ScheduleClipboard& clip = scheduleClipboard();
  if (clip.valid) snprintf(values[(int)Item::Copy], 16, "OK");

  const char* vptrs[(int)Item::COUNT];
  for (int i = 0; i < (int)Item::COUNT; ++i) vptrs[i] = values[i];

  char titleBuf[20];
  snprintf(titleBuf, sizeof(titleBuf), "COPY%s", _editing ? "*" : "");
  r.drawSettingsMenu(_m, titleBuf, labels, vptrs, (int)Item::COUNT, _cursor, _page, 4);
}

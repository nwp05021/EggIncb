#include "RangeEditPage.h"

#include <Arduino.h>

#include "../UiRenderer.h"
#include "../PageManager.h"
#include "../UiApp.h"
#include "../UiModel.h"

static int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
static void toX10Str(int16_t v_x10, char* buf, size_t n) {
  int16_t i = v_x10 / 10;
  int16_t f = abs(v_x10 % 10);
  snprintf(buf, n, "%d.%d", (int)i, (int)f);
}

RangeEditPage::RangeEditPage(UiModel& m, PageManager& mgr, UiApp* app)
  : _m(m), _mgr(mgr), _app(app) {}

void RangeEditPage::bindTable(int16_t* dayTempTable_x10, int16_t* dayHumTable_x10) {
  _pTemp = dayTempTable_x10;
  _pHum  = dayHumTable_x10;
}

const char* RangeEditPage::presetName(RangePreset p) const {
  switch (p) {
    case RangePreset::Custom: return "CUSTOM";
    case RangePreset::D01_18: return "01-18";
    case RangePreset::D19_21: return "19-21";
    case RangePreset::D01_21: return "01-21";
    default: return "?";
  }
}

void RangeEditPage::applyRangePreset(RangePreset p) {
  _preset = p;
  switch (p) {
    case RangePreset::D01_18: _startDay = 1;  _endDay = 18; break;
    case RangePreset::D19_21: _startDay = 19; _endDay = 21; break;
    case RangePreset::D01_21: _startDay = 1;  _endDay = 21; break;
    case RangePreset::Custom:
    default:
      break; // keep
  }
  clampRange();
}

void RangeEditPage::setMode(Mode mode) {
  _mode = mode;
  // sensible defaults
  if (_mode == Mode::Temp) _value_x10 = 370; // 37.0C
  else _value_x10 = 550; // 55.0%
}

void RangeEditPage::onEnter() {
  _cursor = 0;
  _page = 0;
  _editing = false;
  _preset = RangePreset::Custom;
  // initialize from first day
  _startDay = 1;
  _endDay = 1;
  if (_mode == Mode::Temp && _pTemp) _value_x10 = _pTemp[0];
  if (_mode == Mode::Hum  && _pHum)  _value_x10 = _pHum[0];
  clampRange();
  clampValue();
}

void RangeEditPage::clampRange() {
  _startDay = (uint8_t)clampi(_startDay, 1, 21);
  _endDay   = (uint8_t)clampi(_endDay,   1, 21);
  if (_startDay > _endDay) _endDay = _startDay;
}

int RangeEditPage::stepForValue() const {
  return (_mode == Mode::Temp) ? 1 : 10; // 0.1C or 1.0%
}

void RangeEditPage::clampValue() {
  if (_mode == Mode::Temp) {
    _value_x10 = (int16_t)clampi(_value_x10, 300, 450); // 30.0~45.0C
  } else {
    _value_x10 = (int16_t)clampi(_value_x10, 300, 900); // 30.0~90.0%
  }
}

const char* RangeEditPage::title() const {
  return (_mode == Mode::Temp) ? "RANGE TEMP" : "RANGE HUM";
}

void RangeEditPage::onEncoder(int delta) {
  if (delta == 0) return;
  int dir = (delta > 0) ? 1 : -1;

  if (_editing) {
    switch ((Item)_cursor) {
      case Item::Preset: {
        // cycle presets while editing
        uint8_t n = (uint8_t)_preset;
        if (dir > 0) n = (uint8_t)((n + 1) % (uint8_t)RangePreset::COUNT);
        else n = (uint8_t)((n + (uint8_t)RangePreset::COUNT - 1) % (uint8_t)RangePreset::COUNT);
        applyRangePreset((RangePreset)n);
        break;
      }
      case Item::Start:
        _startDay = (uint8_t)clampi((int)_startDay + dir, 1, 21);
        _preset = RangePreset::Custom;
        if (_startDay > _endDay) _endDay = _startDay;
        break;
      case Item::End:
        _endDay = (uint8_t)clampi((int)_endDay + dir, 1, 21);
        _preset = RangePreset::Custom;
        if (_endDay < _startDay) _startDay = _endDay;
        break;
      case Item::Value:
        _value_x10 = (int16_t)(_value_x10 + dir * stepForValue());
        clampValue();
        break;
      default:
        break;
    }
    return;
  }

  // cursor move (stop at ends)
  int next = (int)_cursor + dir;
  if (next < 0) next = 0;
  if (next >= (int)Item::COUNT) next = (int)Item::COUNT - 1;
  _cursor = (uint8_t)next;

  // paging
  int maxPage = ((int)Item::COUNT - 1) / PAGE_SIZE;
  _page = (uint8_t)clampi((int)_page, 0, maxPage);
  int start = _page * PAGE_SIZE;
  int end = start + PAGE_SIZE - 1;
  if ((int)_cursor < start) _page = _cursor / PAGE_SIZE;
  if ((int)_cursor > end) _page = _cursor / PAGE_SIZE;
}

void RangeEditPage::apply() {
  if (!_app) return;
  clampRange();
  clampValue();

  int s = (int)_startDay - 1;
  int e = (int)_endDay - 1;

  if (_mode == Mode::Temp && _pTemp) {
    for (int i = s; i <= e; ++i) _pTemp[i] = _value_x10;
  }
  if (_mode == Mode::Hum && _pHum) {
    for (int i = s; i <= e; ++i) _pHum[i] = _value_x10;
  }
  _app->notifyConfigChanged();
}

void RangeEditPage::onClick() {
  if (_editing) {
    _editing = false;
    return;
  }

  switch ((Item)_cursor) {
    case Item::Preset: {
      // quick cycle preset range
      uint8_t n = (uint8_t)_preset + 1;
      if (n >= (uint8_t)RangePreset::COUNT) n = 0;
      applyRangePreset((RangePreset)n);
      break;
    }
    case Item::Start:
    case Item::End:
    case Item::Value:
      _editing = true;
      break;
    case Item::Apply:
      apply();
      _mgr.pop();
      break;
    case Item::Back:
    default:
      _mgr.pop();
      break;
  }
}

void RangeEditPage::onLongPress() {
  // safety: long press always back
  _mgr.pop();
}

void RangeEditPage::render(UiRenderer& r) {
  static const char* labels[(int)Item::COUNT] = {
    "RNG", "START", "END", "VALUE", "APPLY", "BACK"
  };

  char values[(int)Item::COUNT][16];
  for (int i = 0; i < (int)Item::COUNT; ++i) values[i][0] = 0;

  snprintf(values[(int)Item::Preset], 16, "%s", presetName(_preset));

  snprintf(values[(int)Item::Start], 16, "D%02u", (unsigned)_startDay);
  snprintf(values[(int)Item::End],   16, "D%02u", (unsigned)_endDay);

  if (_mode == Mode::Temp) {
    char tbuf[16];
    toX10Str(_value_x10, tbuf, sizeof(tbuf));
    snprintf(values[(int)Item::Value], 16, "%sC", tbuf);
  } else {
    char hbuf[16];
    toX10Str(_value_x10, hbuf, sizeof(hbuf));
    snprintf(values[(int)Item::Value], 16, "%s%%", hbuf);
  }

  snprintf(values[(int)Item::Apply], 16, "%s", _editing ? "" : "!");

  const char* vptrs[(int)Item::COUNT];
  for (int i = 0; i < (int)Item::COUNT; ++i) vptrs[i] = values[i];

  // Make editing state visible by changing the title slightly.
  char titleBuf[20];
  snprintf(titleBuf, sizeof(titleBuf), "%s%s", title(), _editing ? "*" : "");
  r.drawSettingsMenu(_m, titleBuf, labels, vptrs, (int)Item::COUNT, _cursor, _page, PAGE_SIZE);
}

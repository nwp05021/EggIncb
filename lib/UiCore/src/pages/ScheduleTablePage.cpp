#include "ScheduleTablePage.h"

#include "EditValuePage.h"
#include "RangeEditPage.h"
#include "RangeCopyPage.h"
#include "RangePastePage.h"
#include "ScheduleClipboard.h"
#include "../PageManager.h"
#include "../UiRenderer.h"
#include "../UiApp.h"
#include "../UiModel.h"

#include <Arduino.h>
#include <cstdio>
#include <cmath>

static int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

static void toX10Str(int16_t v_x10, char* buf, size_t n) {
  int16_t i = v_x10 / 10;
  int16_t f = abs(v_x10 % 10);
  snprintf(buf, n, "%d.%d", (int)i, (int)f);
}

ScheduleTablePage::ScheduleTablePage(UiModel& m, PageManager& mgr, UiApp* app)
: _m(m), _mgr(mgr), _app(app) {
  _edit = new EditValuePage(_mgr, _app);
  _rangeEdit = new RangeEditPage(_m, _mgr, _app);
  _copy = new RangeCopyPage(_m, _mgr, _app);
  _paste = new RangePastePage(_m, _mgr, _app);
}

void ScheduleTablePage::bindTable(int16_t* dayTemp_x10, int16_t* dayHum_x10) {
  _pTemp = dayTemp_x10;
  _pHum  = dayHum_x10;

  if (_rangeEdit) _rangeEdit->bindTable(_pTemp, _pHum);
  if (_copy) _copy->bindTable(_pTemp, _pHum);
  if (_paste) _paste->bindTable(_pTemp, _pHum);
}

void ScheduleTablePage::onEnter() {
  _cursor = clampi(_cursor, 0, ITEMS - 1);
  int maxPage = (ITEMS - 1) / PAGE_SIZE;
  _page = clampi(_page, 0, maxPage);
}

void ScheduleTablePage::onEncoder(int delta) {
  if (delta == 0) return;
  int dir = (delta > 0) ? 1 : -1;

  _cursor += dir;
  // clamp (no wrap)
  if (_cursor < 0) _cursor = 0;
  if (_cursor >= ITEMS) _cursor = ITEMS - 1;

  int maxPage = (ITEMS - 1) / PAGE_SIZE;
  _page = clampi(_page, 0, maxPage);

  int start = _page * PAGE_SIZE;
  int end = start + PAGE_SIZE - 1;
  if (_cursor < start) _page = _cursor / PAGE_SIZE;
  if (_cursor > end) _page = _cursor / PAGE_SIZE;
}

void ScheduleTablePage::onClick() {
  if (!_pTemp || !_pHum) {
    _mgr.pop();
    return;
  }

  // BACK item
  if (_cursor == ITEMS - 1) {
    _mgr.pop();
    return;
  }

  // --- Tools (top section) ---
  if (_cursor < TOOL_COUNT) {
    switch (_cursor) {
      case TOOL_RANGE_TEMP:
        _rangeEdit->setMode(RangeEditPage::Mode::Temp);
        _mgr.push(_rangeEdit);
        return;
      case TOOL_RANGE_HUM:
        _rangeEdit->setMode(RangeEditPage::Mode::Hum);
        _mgr.push(_rangeEdit);
        return;
      case TOOL_COPY:
        _mgr.push(_copy);
        return;
      case TOOL_PASTE:
        _mgr.push(_paste);
        return;
      case TOOL_PRESET_19_21_HUM_UP:
        // Preset: boost HUM for day 19~21 (+10%)
        for (int d = 18; d <= 20; ++d) {
          int v = (int)_pHum[d] + 100; // +10.0%
          if (v > 900) v = 900;
          if (v < 300) v = 300;
          _pHum[d] = (int16_t)v;
        }
        if (_app) _app->notifyConfigChanged();
        return;
      default:
        return;
    }
  }

  const int idx = _cursor - TOOL_COUNT;
  const int dayIdx = idx / 2;      // 0..20

  if (dayIdx < 0 || dayIdx >= DAYS) return;

  if (((idx % 2) == 0)) {
    // temp
    _edit->configure("TEMP", "C", EditType::I16_X10, &_pTemp[dayIdx], 350, 390, 1, 5);
  } else {
    // hum
    _edit->configure("HUM", "%", EditType::I16_X10, &_pHum[dayIdx], 300, 900, 5, 20);
  }

  _mgr.push(_edit);
}

void ScheduleTablePage::onLongPress() {
  _mgr.pop();
}

void ScheduleTablePage::render(UiRenderer& r) {
  static char labels[ITEMS][12];
  static char values[ITEMS][16];

  for (int i = 0; i < ITEMS; ++i) {
    labels[i][0] = 0;
    values[i][0] = 0;
  }

  // tools
  snprintf(labels[TOOL_RANGE_TEMP], sizeof(labels[TOOL_RANGE_TEMP]), "R-TEMP");
  snprintf(labels[TOOL_RANGE_HUM],  sizeof(labels[TOOL_RANGE_HUM]),  "R-HUM");
  snprintf(labels[TOOL_COPY],       sizeof(labels[TOOL_COPY]),       "COPY");
  snprintf(labels[TOOL_PASTE],      sizeof(labels[TOOL_PASTE]),      "PASTE");
  snprintf(labels[TOOL_PRESET_19_21_HUM_UP], sizeof(labels[TOOL_PRESET_19_21_HUM_UP]), "H+19-21");

  ScheduleClipboard& clip = scheduleClipboard();
  snprintf(values[TOOL_RANGE_TEMP], sizeof(values[TOOL_RANGE_TEMP]), "");
  snprintf(values[TOOL_RANGE_HUM],  sizeof(values[TOOL_RANGE_HUM]),  "");
  snprintf(values[TOOL_COPY],       sizeof(values[TOOL_COPY]),       clip.valid ? "OK" : "");
  snprintf(values[TOOL_PASTE],      sizeof(values[TOOL_PASTE]),      clip.valid ? "L%u" : "-", (unsigned)clip.len);
  snprintf(values[TOOL_PRESET_19_21_HUM_UP], sizeof(values[TOOL_PRESET_19_21_HUM_UP]), "!");

  // day rows
  if (_pTemp && _pHum) {
    for (int d = 0; d < DAYS; ++d) {
      const int base = TOOL_COUNT + d * 2;
      snprintf(labels[base + 0], sizeof(labels[base + 0]), "D%02d-T", d + 1);
      snprintf(labels[base + 1], sizeof(labels[base + 1]), "D%02d-H", d + 1);
      toX10Str(_pTemp[d], values[base + 0], sizeof(values[base + 0]));
      toX10Str(_pHum[d],  values[base + 1], sizeof(values[base + 1]));
    }
  } else {
    for (int d = 0; d < DAYS; ++d) {
      const int base = TOOL_COUNT + d * 2;
      snprintf(labels[base + 0], sizeof(labels[base + 0]), "D%02d-T", d + 1);
      snprintf(labels[base + 1], sizeof(labels[base + 1]), "D%02d-H", d + 1);
      snprintf(values[base + 0], sizeof(values[base + 0]), "--.-");
      snprintf(values[base + 1], sizeof(values[base + 1]), "--.-");
    }
  }

  snprintf(labels[ITEMS - 1], sizeof(labels[ITEMS - 1]), "BACK");

  const char* labelPtrs[ITEMS];
  const char* valuePtrs[ITEMS];
  for (int i = 0; i < ITEMS; ++i) {
    labelPtrs[i] = labels[i];
    valuePtrs[i] = values[i];
  }

  r.drawSettingsMenu(_m, "스케줄표", labelPtrs, valuePtrs, ITEMS, _cursor, _page, PAGE_SIZE);
}

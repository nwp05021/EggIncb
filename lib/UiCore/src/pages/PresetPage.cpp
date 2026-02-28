#include "PresetPage.h"
#include "ConfirmPage.h"
#include "../UiRenderer.h"
#include "../PageManager.h"
#include "../UiApp.h"
#include <Arduino.h>

static int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

// Keep these IDs in sync with src/config/Defaults.h
static constexpr uint8_t PRESET_CHICKEN_STD   = 0;
static constexpr uint8_t PRESET_CHICKEN_HATCH = 1;
static constexpr uint8_t PRESET_CLEAR_CUSTOM  = 2;
static constexpr uint8_t PRESET_COUNT         = 3;

PresetPage::PresetPage(PageManager& mgr, UiApp* app)
: _mgr(mgr), _app(app) {
  _confirm = new ConfirmPage(_mgr);
}

void PresetPage::bindPreset(uint8_t* presetId,
                            int16_t* dayTempTable_x10,
                            int16_t* dayHumTable_x10) {
  _pPresetId = presetId;
  _pDayTemp  = dayTempTable_x10;
  _pDayHum   = dayHumTable_x10;
}

void PresetPage::ensureVisible() {
  int maxPage = (PRESET_COUNT - 1) / PAGE_SIZE;
  _page = clampi(_page, 0, maxPage);

  int start = _page * PAGE_SIZE;
  int end = start + PAGE_SIZE - 1;
  if (_cursor < start) _page = _cursor / PAGE_SIZE;
  if (_cursor > end) _page = _cursor / PAGE_SIZE;
}

void PresetPage::onEnter() {
  _cursor = 0;
  if (_pPresetId) _cursor = clampi((int)(*_pPresetId), 0, (int)PRESET_COUNT - 1);
  ensureVisible();
}

void PresetPage::onEncoder(int delta) {
  if (delta == 0) return;
  int dir = (delta > 0) ? 1 : -1;

  // Clamp at edges (no wrap)
  int next = clampi(_cursor + dir, 0, (int)PRESET_COUNT - 1);
  if (next == _cursor) return;
  _cursor = next;
  ensureVisible();
}

void PresetPage::applyPreset(uint8_t id) {
  if (!_pPresetId || !_pDayTemp || !_pDayHum) return;

  *_pPresetId = id;

  switch (id) {
    case PRESET_CHICKEN_STD:
      for (int i = 0; i < 21; ++i) { _pDayTemp[i] = 375; _pDayHum[i] = 550; }
      break;
    case PRESET_CHICKEN_HATCH:
      for (int i = 0; i < 21; ++i) {
        if (i < 18) { _pDayTemp[i] = 375; _pDayHum[i] = 550; }
        else        { _pDayTemp[i] = 372; _pDayHum[i] = 700; }
      }
      break;
    case PRESET_CLEAR_CUSTOM:
    default:
      for (int i = 0; i < 21; ++i) { _pDayTemp[i] = 0; _pDayHum[i] = 0; }
      break;
  }

  if (_app) _app->notifyConfigChanged();
}

static void preset_apply_thunk_0();
static void preset_apply_thunk_1();
static void preset_apply_thunk_2();

// We use a static pointer for ConfirmPage callback (void(*)()).
static PresetPage* s_self = nullptr;
static void preset_apply_thunk_0() { if (s_self) s_self->applyPreset(PRESET_CHICKEN_STD); }
static void preset_apply_thunk_1() { if (s_self) s_self->applyPreset(PRESET_CHICKEN_HATCH); }
static void preset_apply_thunk_2() { if (s_self) s_self->applyPreset(PRESET_CLEAR_CUSTOM); }

void PresetPage::onClick() {
  // Confirm before overwriting the table.
  s_self = this;

  void (*fn)() = preset_apply_thunk_0;
  const char* name = "CHICKEN";

  if (_cursor == PRESET_CHICKEN_STD) {
    fn = preset_apply_thunk_0;
    name = "CHICKEN STD";
  } else if (_cursor == PRESET_CHICKEN_HATCH) {
    fn = preset_apply_thunk_1;
    name = "HATCH BOOST";
  } else {
    fn = preset_apply_thunk_2;
    name = "CLEAR";
  }

  _confirm->configure("PRESET", name, "Apply preset?", fn);
  _mgr.push(_confirm);
}

void PresetPage::onLongPress() { _mgr.pop(); }

void PresetPage::render(UiRenderer& r) {
  static const char* labels[] = {
    "CHICKEN STD",
    "HATCH BOOST",
    "CLEAR",
  };
  static char values[PRESET_COUNT][16];
  for (int i = 0; i < (int)PRESET_COUNT; ++i) values[i][0] = 0;

  // Mark selected
  if (_pPresetId && *_pPresetId < PRESET_COUNT) {
    snprintf(values[*_pPresetId], 16, "*");
  }

  const char* valuePtrs[PRESET_COUNT];
  for (int i = 0; i < (int)PRESET_COUNT; ++i) valuePtrs[i] = values[i];

  // Reuse settings menu renderer for a consistent list UI.
  UiModel& m = _app->model();
  r.drawSettingsMenu(m, "PRESET", labels, valuePtrs,
                     (int)PRESET_COUNT, _cursor, _page, PAGE_SIZE);
}

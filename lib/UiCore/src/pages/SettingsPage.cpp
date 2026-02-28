#include "SettingsPage.h"

#include "EditValuePage.h"
#include "ConfirmPage.h"
#include "ScheduleTablePage.h"
#include "PresetPage.h"

#include "../UiRenderer.h"
#include "../PageManager.h"
#include "../UiApp.h"

#include <Arduino.h>

static int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

SettingsPage::SettingsPage(UiModel& m, PageManager& mgr, UiApp* app)
  : _m(m), _mgr(mgr), _app(app) {
  _edit    = new EditValuePage(_mgr, _app);
  _confirm = new ConfirmPage(_mgr);
  _table   = new ScheduleTablePage(_m, _mgr, _app);
  _preset  = new PresetPage(_mgr, _app);
}

void SettingsPage::bindConfig(uint16_t* startYear,
                              uint8_t* startMonth,
                              uint8_t* startDay,
                              uint8_t* presetId,
                              int16_t* dayTempTable_x10,
                              int16_t* dayHumTable_x10,
                              int16_t* tempHyst_x10,
                              int16_t* humHyst_x10,
                              uint16_t* motorOnSec,
                              uint16_t* motorOffMin,
                              bool* heaterEnabled,
                              bool* motorEnabled,
                              bool* fanEnabled,
                              bool* humidifierEnabled) {
  _pStartYear  = startYear;
  _pStartMonth = startMonth;
  _pStartDay   = startDay;

  _pPresetId = presetId;
  _pDayTempTable = dayTempTable_x10;
  _pDayHumTable  = dayHumTable_x10;

  _pTempHyst = tempHyst_x10;
  _pHumHyst  = humHyst_x10;

  _pOnSec   = motorOnSec;
  _pOffMin  = motorOffMin;

  _pHeaterEn = heaterEnabled;
  _pMotorEn  = motorEnabled;
  _pFanEn    = fanEnabled;
  _pHumidEn  = humidifierEnabled;

  // also bind sub pages
  if (_preset) _preset->bindPreset(_pPresetId, _pDayTempTable, _pDayHumTable);
  if (_table)  _table->bindTable(_pDayTempTable, _pDayHumTable);
}

void SettingsPage::syncFromConfig() {
  if (!_pStartYear) return;

  _m.startYear  = *_pStartYear;
  _m.startMonth = *_pStartMonth;
  _m.startDay   = *_pStartDay;

  if (_pPresetId) _m.presetId = *_pPresetId;

  if (_pTempHyst) _m.tempHyst_x10 = *_pTempHyst;
  if (_pHumHyst)  _m.humHyst_x10  = *_pHumHyst;

  if (_pOnSec)  _m.motorOnSec  = *_pOnSec;
  if (_pOffMin) _m.motorOffMin = *_pOffMin;

  if (_pHeaterEn) _m.heaterEnabled = *_pHeaterEn;
  if (_pMotorEn)  _m.motorEnabled  = *_pMotorEn;
  if (_pFanEn)    _m.fanEnabled    = *_pFanEn;
  if (_pHumidEn)  _m.humidifierEnabled = *_pHumidEn;
}

void SettingsPage::ensureVisible() {
  int maxPage = (itemCount() - 1) / PAGE_SIZE;
  _page = clampi(_page, 0, maxPage);

  int start = _page * PAGE_SIZE;
  int end = start + PAGE_SIZE - 1;
  if (_cursor < start) _page = _cursor / PAGE_SIZE;
  if (_cursor > end)   _page = _cursor / PAGE_SIZE;
}

void SettingsPage::onEnter() {
  syncFromConfig();
  ensureVisible();
}

void SettingsPage::onEncoder(int delta) {
  if (delta == 0) return;
  int dir = (delta > 0) ? 1 : -1;

  // Clamp at edges (no wrap).
  int next = clampi(_cursor + dir, 0, itemCount() - 1);
  if (next == _cursor) return;
  _cursor = next;
  ensureVisible();
}

extern void ui_factory_reset();
extern void ui_provisioning_reset();
extern void ui_time_sync();

void SettingsPage::onClick() {
  syncFromConfig();
  SettingItem item = (SettingItem)_cursor;

  switch (item) {
    case SettingItem::StartYear:
      _edit->configure("START-Y", "", EditType::U16, _pStartYear, 2020, 2035, 1, 5);
      _mgr.push(_edit);
      break;
    case SettingItem::StartMonth:
      _edit->configure("START-M", "", EditType::U8, _pStartMonth, 1, 12, 1, 1);
      _mgr.push(_edit);
      break;
    case SettingItem::StartDay:
      _edit->configure("START-D", "", EditType::U8, _pStartDay, 1, 31, 1, 1);
      _mgr.push(_edit);
      break;

    case SettingItem::Preset:
      if (_preset) {
        _preset->bindPreset(_pPresetId, _pDayTempTable, _pDayHumTable);
        _mgr.push(_preset);
      }
      break;

    case SettingItem::ScheduleTable:
      if (_table) {
        _table->bindTable(_pDayTempTable, _pDayHumTable);
        _mgr.push(_table);
      }
      break;

    case SettingItem::TempHyst:
      _edit->configure("HYST", "C", EditType::I16_X10, _pTempHyst, 1, 50, 1, 5);
      _mgr.push(_edit);
      break;

    case SettingItem::HumHyst:
      _edit->configure("HHYS", "%", EditType::I16_X10, _pHumHyst, 5, 300, 5, 20);
      _mgr.push(_edit);
      break;

    case SettingItem::MotorOnSec:
      _edit->configure("M-ON", "s", EditType::U16, _pOnSec, 1, 120, 1, 5);
      _mgr.push(_edit);
      break;

    case SettingItem::MotorOffMin:
      _edit->configure("M-OFF", "m", EditType::U16, _pOffMin, 1, 240, 1, 10);
      _mgr.push(_edit);
      break;

    case SettingItem::HeaterEnable:
      _edit->configure("HEAT", "", EditType::Bool, _pHeaterEn, 0, 1, 1, 1);
      _mgr.push(_edit);
      break;
    case SettingItem::MotorEnable:
      _edit->configure("MOTOR", "", EditType::Bool, _pMotorEn, 0, 1, 1, 1);
      _mgr.push(_edit);
      break;
    case SettingItem::FanEnable:
      _edit->configure("FAN", "", EditType::Bool, _pFanEn, 0, 1, 1, 1);
      _mgr.push(_edit);
      break;
    case SettingItem::HumidEnable:
      _edit->configure("HUMID", "", EditType::Bool, _pHumidEn, 0, 1, 1, 1);
      _mgr.push(_edit);
      break;

    case SettingItem::TimeSync:
      _confirm->configure("TIME", "Time Sync", "Sync now?", ui_time_sync);
      _mgr.push(_confirm);
      break;
    case SettingItem::ProvisioningReset:
      _confirm->configure("PROV", "WiFi clear", "BLE setup again?", ui_provisioning_reset);
      _mgr.push(_confirm);
      break;
    case SettingItem::FactoryReset:
      _confirm->configure("RESET", "Factory reset", "Proceed?", ui_factory_reset);
      _mgr.push(_confirm);
      break;

    case SettingItem::Back:
    default:
      _mgr.pop();
      break;
  }
}

void SettingsPage::onLongPress() { _mgr.pop(); }

static void u16ToStr(char* out, size_t n, uint16_t v) { snprintf(out, n, "%u", (unsigned)v); }

void SettingsPage::formatValues(char out[][16], int count) {
  for (int i = 0; i < count; ++i) out[i][0] = 0;

  u16ToStr(out[(int)SettingItem::StartYear], 16, _m.startYear);
  snprintf(out[(int)SettingItem::StartMonth], 16, "%02u", (unsigned)_m.startMonth);
  snprintf(out[(int)SettingItem::StartDay], 16, "%02u", (unsigned)_m.startDay);

  // Preset id display (0..)
  snprintf(out[(int)SettingItem::Preset], 16, "#%u", (unsigned)_m.presetId);
  snprintf(out[(int)SettingItem::ScheduleTable], 16, ">>");

  // hysteresis
  {
    // use renderer helper style: x10 -> string
    int16_t v = _m.tempHyst_x10;
    int16_t i = v / 10;
    int16_t f = abs(v % 10);
    snprintf(out[(int)SettingItem::TempHyst], 16, "%d.%d", (int)i, (int)f);
  }
  {
    int16_t v = _m.humHyst_x10;
    int16_t i = v / 10;
    int16_t f = abs(v % 10);
    snprintf(out[(int)SettingItem::HumHyst], 16, "%d.%d", (int)i, (int)f);
  }

  snprintf(out[(int)SettingItem::MotorOnSec], 16, "%u", (unsigned)_m.motorOnSec);
  snprintf(out[(int)SettingItem::MotorOffMin], 16, "%u", (unsigned)_m.motorOffMin);

  snprintf(out[(int)SettingItem::HeaterEnable], 16, "%s", _m.heaterEnabled ? "ON" : "OFF");
  snprintf(out[(int)SettingItem::MotorEnable], 16, "%s", _m.motorEnabled ? "ON" : "OFF");
  snprintf(out[(int)SettingItem::FanEnable], 16, "%s", _m.fanEnabled ? "ON" : "OFF");
  snprintf(out[(int)SettingItem::HumidEnable], 16, "%s", _m.humidifierEnabled ? "ON" : "OFF");

  snprintf(out[(int)SettingItem::TimeSync], 16, "!");
  snprintf(out[(int)SettingItem::ProvisioningReset], 16, "!");
  snprintf(out[(int)SettingItem::FactoryReset], 16, "!");
  out[(int)SettingItem::Back][0] = 0;
}

void SettingsPage::render(UiRenderer& r) {
  static const char* labels[] = {
    "SY", "SM", "SD",
    "PRESET",
    "TABLE",
    "HYST", "HHYS",
    "M-ON", "M-OFF",
    "HEAT", "MOTOR", "FAN", "HUMID",
    "TIME SYNC",
    "PROV",
    "FACTORY",
    "BACK",
  };

  static_assert((int)SettingItem::COUNT == (sizeof(labels) / sizeof(labels[0])),
                "Settings labels count mismatch");

  static char values[(int)SettingItem::COUNT][16];
  syncFromConfig();
  formatValues(values, (int)SettingItem::COUNT);

  const char* valuePtrs[(int)SettingItem::COUNT];
  for (int i = 0; i < (int)SettingItem::COUNT; ++i) valuePtrs[i] = values[i];

  r.drawSettingsMenu(_m, "설정", labels, valuePtrs,
                     (int)SettingItem::COUNT, _cursor, _page, PAGE_SIZE);
}

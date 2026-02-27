#include "SettingsPage.h"
#include "EditValuePage.h"
#include "ConfirmPage.h"
#include "../UiRenderer.h"
#include "../PageManager.h"
#include "../UiApp.h"
#include <Arduino.h>

static int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
static void toX10Str(int16_t v_x10, char* buf, size_t n) {
  int16_t i = v_x10 / 10;
  int16_t f = abs(v_x10 % 10);
  snprintf(buf, n, "%d.%d", i, f);
}

SettingsPage::SettingsPage(UiModel& m, PageManager& mgr, UiApp* app)
: _m(m), _mgr(mgr), _app(app) {
  _edit = new EditValuePage(_mgr, _app);
  _confirm = new ConfirmPage(_mgr);
}

void SettingsPage::bindConfig(uint8_t* scheduleMode,
                              uint16_t* startYear,
                              uint8_t* startMonth,
                              uint8_t* startDay,
                              int16_t* targetTemp_x10,
                              int16_t* tempHyst_x10,
                              int16_t* targetHum_x10,
                              int16_t* humHyst_x10,
                              uint16_t* motorOnSec,
                              uint16_t* motorOffMin,
                              bool* heaterEnabled,
                              bool* motorEnabled,
                              bool* fanEnabled,
                              bool* humidifierEnabled) {
  _pScheduleMode = scheduleMode;
  _pStartYear = startYear;
  _pStartMonth = startMonth;
  _pStartDay = startDay;

  _pTargetTemp = targetTemp_x10;
  _pTempHyst = tempHyst_x10;

  _pTargetHum = targetHum_x10;
  _pHumHyst = humHyst_x10;

  _pOnSec = motorOnSec;
  _pOffMin = motorOffMin;

  _pHeaterEn = heaterEnabled;
  _pMotorEn = motorEnabled;
  _pFanEn = fanEnabled;
  _pHumidEn = humidifierEnabled;
}

void SettingsPage::syncFromConfig() {
  if (!_pTargetTemp) return;

  _m.scheduleMode = *_pScheduleMode;

  _m.startYear = *_pStartYear;
  _m.startMonth = *_pStartMonth;
  _m.startDay = *_pStartDay;

  _m.targetTemp_x10 = *_pTargetTemp;
  _m.tempHyst_x10 = *_pTempHyst;

  _m.targetHum_x10 = *_pTargetHum;
  _m.humHyst_x10 = *_pHumHyst;

  _m.motorOnSec = *_pOnSec;
  _m.motorOffMin = *_pOffMin;

  _m.heaterEnabled = *_pHeaterEn;
  _m.motorEnabled = *_pMotorEn;
  _m.fanEnabled = *_pFanEn;
  _m.humidifierEnabled = *_pHumidEn;
}

void SettingsPage::ensureVisible() {
  int maxPage = (itemCount() - 1) / PAGE_SIZE;
  _page = clampi(_page, 0, maxPage);

  int start = _page * PAGE_SIZE;
  int end = start + PAGE_SIZE - 1;
  if (_cursor < start) _page = _cursor / PAGE_SIZE;
  if (_cursor > end) _page = _cursor / PAGE_SIZE;
}

void SettingsPage::onEnter() {
  syncFromConfig();
  ensureVisible();
}

void SettingsPage::onEncoder(int delta)
{
    if (delta == 0) return;

    int dir = (delta > 0) ? 1 : -1;

    _cursor += dir;

    if (_cursor < 0)
        _cursor = 0;

    if (_cursor >= itemCount())
        _cursor = itemCount() - 1;

    ensureVisible();
}

extern void ui_factory_reset();       // defined in main.cpp
extern void ui_provisioning_reset();  // defined in main.cpp
extern void ui_time_sync();           // defined in main.cpp

void SettingsPage::onClick() {
  syncFromConfig();
  SettingItem item = (SettingItem)_cursor;

  switch (item) {
    case SettingItem::ScheduleMode:
      // 0=AUTO, 1=MANUAL
      _edit->configure("SCHED", "", EditType::U8, _pScheduleMode, 0, 1, 1, 1);
      _mgr.push(_edit);
      break;

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

    case SettingItem::TargetTemp:
      _edit->configure("TEMP", "C", EditType::I16_X10, _pTargetTemp, 350, 390, 1, 10);
      _mgr.push(_edit);
      break;

    case SettingItem::TempHyst:
      _edit->configure("HYST", "C", EditType::I16_X10, _pTempHyst, 1, 30, 1, 5);
      _mgr.push(_edit);
      break;

    case SettingItem::TargetHum:
      _edit->configure("HUM", "%", EditType::I16_X10, _pTargetHum, 300, 900, 5, 20);
      _mgr.push(_edit);
      break;

    case SettingItem::HumHyst:
      _edit->configure("HHYS", "%", EditType::I16_X10, _pHumHyst, 5, 200, 5, 20);
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
      _confirm->configure("TIME-SYNC", "Time Sync", "Time Sync?", ui_time_sync);
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

void SettingsPage::formatValues(char out[][16], int count) {
  for (int i = 0; i < count; ++i) out[i][0] = 0;

  snprintf(out[(int)SettingItem::ScheduleMode], 16, "%s", (_m.scheduleMode == 0) ? "AUTO" : "MAN");

  snprintf(out[(int)SettingItem::StartYear], 16, "%u", (unsigned)_m.startYear);
  snprintf(out[(int)SettingItem::StartMonth], 16, "%02u", (unsigned)_m.startMonth);
  snprintf(out[(int)SettingItem::StartDay], 16, "%02u", (unsigned)_m.startDay);

  toX10Str(_m.targetTemp_x10, out[(int)SettingItem::TargetTemp], 16);
  toX10Str(_m.tempHyst_x10, out[(int)SettingItem::TempHyst], 16);

  toX10Str(_m.targetHum_x10, out[(int)SettingItem::TargetHum], 16);
  toX10Str(_m.humHyst_x10, out[(int)SettingItem::HumHyst], 16);

  snprintf(out[(int)SettingItem::MotorOnSec], 16, "%u", (unsigned)_m.motorOnSec);
  snprintf(out[(int)SettingItem::MotorOffMin], 16, "%u", (unsigned)_m.motorOffMin);

  snprintf(out[(int)SettingItem::HeaterEnable], 16, "%s", _m.heaterEnabled ? "ON" : "OFF");
  snprintf(out[(int)SettingItem::MotorEnable], 16, "%s", _m.motorEnabled ? "ON" : "OFF");
  snprintf(out[(int)SettingItem::FanEnable], 16, "%s", _m.fanEnabled ? "ON" : "OFF");
  snprintf(out[(int)SettingItem::HumidEnable], 16, "%s", _m.humidifierEnabled ? "ON" : "OFF");

  snprintf(out[(int)SettingItem::ProvisioningReset], 16, "!");

  snprintf(out[(int)SettingItem::FactoryReset], 16, "!");
  out[(int)SettingItem::Back][0] = 0;
}

void SettingsPage::render(UiRenderer& r) {
  // static const char* labels[] = {
  //   "스케줄",
  //   "시작년도",
  //   "시작월",
  //   "시작일",
  //   "목표온도",
  //   "온도히스",
  //   "목표습도",
  //   "습도히스",
  //   "모터ON",
  //   "모터OFF",
  //   "히터사용",
  //   "모터사용",
  //   "팬사용",
  //   "가습사용",
  //   "초기화",
  //   "뒤로"
  // };
  static const char* labels[] = {
    "SCHED",

    "SY",
    "SM",
    "SD",

    "TEMP",
    "HYST",
    "HUM",
    "HHYS",

    "M-ON",
    "M-OFF",

    "HEAT",
    "MOTOR",
    "FAN",
    "HUMID",

    "TIME SYNC",      // ✅ 추가 (시간 동기화)
    "PROV",           // ✅ 추가 (ProvisioningReset)
    "FACTORY RESET",  // FactoryReset
    "BACK"
  };  


  static_assert((int)SettingItem::COUNT == (sizeof(labels) / sizeof(labels[0])),
              "Settings labels count mismatch with SettingItem::COUNT");

  static char values[(int)SettingItem::COUNT][16];
  syncFromConfig();
  formatValues(values, (int)SettingItem::COUNT);

  const char* valuePtrs[(int)SettingItem::COUNT];
  for (int i = 0; i < (int)SettingItem::COUNT; ++i) valuePtrs[i] = values[i];

  r.drawSettingsMenu(_m, "설정", labels, valuePtrs, (int)SettingItem::COUNT, _cursor, _page, PAGE_SIZE);
}

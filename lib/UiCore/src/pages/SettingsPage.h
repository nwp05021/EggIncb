#pragma once
#include "../Page.h"
#include "../UiModel.h"
#include <stdint.h>

class PageManager;
class UiApp;
class EditValuePage;
class ConfirmPage;
class ScheduleTablePage;
class PresetPage;

enum class SettingItem : uint8_t {
  StartYear,
  StartMonth,
  StartDay,

  Preset,

  ScheduleTable,

  TempHyst,
  HumHyst,

  MotorOnSec,
  MotorOffMin,

  HeaterEnable,
  MotorEnable,
  FanEnable,
  HumidEnable,

  TimeSync,
  ProvisioningReset,
  FactoryReset,
  Back,
  COUNT
};

class SettingsPage : public Page {
public:
  SettingsPage(UiModel& m, PageManager& mgr, UiApp* app);

  void onEnter() override;
  void onEncoder(int delta) override;
  void onClick() override;
  void onLongPress() override;
  void render(class UiRenderer& r) override;

  void bindConfig(uint16_t* startYear,
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
                  bool* humidifierEnabled);

private:
  UiModel& _m;
  PageManager& _mgr;
  UiApp* _app;

  int _cursor = 0;
  int _page = 0;
  static constexpr int PAGE_SIZE = 4;

  // config pointers
  uint8_t* _pDay = nullptr; // legacy/backup day (unused for now)
  uint16_t* _pStartYear = nullptr;
  uint8_t*  _pStartMonth = nullptr;
  uint8_t*  _pStartDay = nullptr;
  uint8_t*  _pPresetId = nullptr;

  int16_t*  _pDayTempTable = nullptr;
  int16_t*  _pDayHumTable  = nullptr;

  int16_t*  _pTempHyst   = nullptr;
  int16_t*  _pHumHyst   = nullptr;

  uint16_t* _pOnSec  = nullptr;
  uint16_t* _pOffMin = nullptr;

  bool*     _pHeaterEn = nullptr;
  bool*     _pMotorEn  = nullptr;
  bool*     _pFanEn    = nullptr;
  bool*     _pHumidEn  = nullptr;

  EditValuePage* _edit = nullptr;
  ConfirmPage* _confirm = nullptr;
  ScheduleTablePage* _table = nullptr;
  PresetPage* _preset = nullptr;
  class ScheduleTablePage* _sched = nullptr;

  void syncFromConfig();
  void ensureVisible();
  int itemCount() const { return (int)SettingItem::COUNT; }

  void formatValues(char out[][16], int count);
};

#pragma once
#include "../Page.h"
#include "../UiModel.h"
#include <stdint.h>

class PageManager;
class UiApp;
class EditValuePage;
class ConfirmPage;

enum class SettingItem : uint8_t {
  ScheduleMode,
  IncubationDay,

  StartYear,
  StartMonth,
  StartDay,

  TargetTemp,
  TempHyst,

  TargetHum,
  HumHyst,

  MotorOnSec,
  MotorOffMin,

  HeaterEnable,
  MotorEnable,
  FanEnable,
  HumidEnable,

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

  void bindConfig(uint8_t* scheduleMode,
                  uint8_t* incubationDay,
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
                  bool* humidifierEnabled);

private:
  UiModel& _m;
  PageManager& _mgr;
  UiApp* _app;

  int _cursor = 0;
  int _page = 0;
  static constexpr int PAGE_SIZE = 5;

  // config pointers
  uint8_t* _pScheduleMode = nullptr;
  uint8_t* _pDay = nullptr;
  uint16_t* _pStartYear = nullptr;
  uint8_t*  _pStartMonth = nullptr;
  uint8_t*  _pStartDay = nullptr;

  int16_t*  _pTargetTemp = nullptr;
  int16_t*  _pTempHyst   = nullptr;

  int16_t*  _pTargetHum = nullptr;
  int16_t*  _pHumHyst   = nullptr;

  uint16_t* _pOnSec  = nullptr;
  uint16_t* _pOffMin = nullptr;

  bool*     _pHeaterEn = nullptr;
  bool*     _pMotorEn  = nullptr;
  bool*     _pFanEn    = nullptr;
  bool*     _pHumidEn  = nullptr;

  EditValuePage* _edit = nullptr;
  ConfirmPage* _confirm = nullptr;

  void syncFromConfig();
  void ensureVisible();
  int itemCount() const { return (int)SettingItem::COUNT; }

  void formatValues(char out[][16], int count);
};

#pragma once
#include <stdint.h>
#include "UiModel.h"
#include "UiRenderer.h"
#include "PageManager.h"

struct UiCallbacks {
  void (*onConfigChanged)() = nullptr;  // markPersistDirty
};

class UiApp {
public:
  UiApp();
  ~UiApp();

  void begin(UiRenderer* renderer, UiCallbacks cb);

  UiModel& model() { return _model; }
  const UiModel& model() const { return _model; }

  // Settings: start date + hysteresis + motor timing + actuator enables.
  // Targets are derived from the incubation schedule table (day -> temp/hum).
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

  void notifyConfigChanged();

  // input events
  void onEncoder(int delta);
  void onClick();
  void onLongPress();
  void onVeryLongPress();

  void tick(uint32_t nowMs);
  void render();

  void requestFactoryReset();
  bool takeFactoryResetRequest();

private:
  UiRenderer* _r = nullptr;
  UiCallbacks _cb{};
  UiModel _model{};
  PageManager _mgr;

  // config pointers (owned by main)
  uint8_t* _pDay = nullptr; // legacy/backup day (optional)
  uint16_t* _pStartYear = nullptr;
  uint8_t*  _pStartMonth = nullptr;
  uint8_t*  _pStartDay = nullptr;
  uint8_t*  _pPresetId = nullptr;

  int16_t*  _pTempHyst   = nullptr;
  int16_t*  _pHumHyst    = nullptr;

  uint16_t* _pOnSec  = nullptr;
  uint16_t* _pOffMin = nullptr;

  bool*     _pHeaterEn = nullptr;
  bool*     _pMotorEn  = nullptr;
  bool*     _pFanEn    = nullptr;
  bool*     _pHumidEn  = nullptr;

  Page* _main = nullptr;
  class SettingsPage* _settings = nullptr;

  uint32_t _lastBlinkMs = 0;
  bool _factoryResetRequested = false;

  void updateBlink(uint32_t nowMs);
  void syncModelFromConfig();
};

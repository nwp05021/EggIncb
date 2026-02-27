#pragma once

#include <Arduino.h>
#include <UiApp.h>
#include <UiRenderer.h>

#include "app/system/PersistedData.h"
#include "app/system/SettingsStore.h"
#include "app/system/ProvisioningManager.h"
#include "app/system/TimeManager.h"

#include "app/services/EnvSensor_AHT.h"
#include "app/controllers/IncubatorController.h"
#include "hal/RelayHal.h"

class IncubatorApp {
public:
  void begin();
  void tick();

  void requestFactoryReset();
  void requestProvisioningReset();
  void requestTimeSync();

  void syncBoolsToCfg();

private:
  SettingsStore _settings;
  ProvisioningManager _prov;
  TimeManager _timeMgr;
  EnvSensor_AHT _env;
  IncubatorController _incubator;
  RelayHal _relay;
  UiRenderer _renderer;
  UiApp _ui;

  void computeElapsedDay();
  void applyRuntimeDay();

  void syncCfgToBools();

  PersistedData _cfg{};
  bool _heaterEn = true;
  bool _motorEn  = true;
  bool _fanEn    = true;
  bool _humidEn  = false;

  bool _factoryResetRequested = false;
};
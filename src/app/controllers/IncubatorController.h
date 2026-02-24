#pragma once
#include <stdint.h>
#include "../system/PersistedData.h"
#include "TempControl.h"
#include "HumidityControl.h"
#include "MotorSchedule.h"
#include "IncubationSchedule.h"
#include "VentilationTimer.h"

struct EffectiveTargets {
  int16_t temp_x10;
  int16_t hum_x10;
};

class IncubatorController {
public:
  void begin(const PersistedData& cfg, uint32_t bootMs);
  void applyConfig(const PersistedData& cfg);

  void update(float currentTempC, float currentHumRH, bool sensorOk, uint32_t nowMs);

  bool heaterState() const { return _heater; }
  bool motorState() const { return _motor; }
  bool fanState() const { return _fan; }
  bool humidifierState() const { return _humid; }

  int16_t currentTemp_x10() const { return _currentTemp_x10; }
  int16_t currentHum_x10() const { return _currentHum_x10; }

  EffectiveTargets targets() const { return _targets; }

  bool alarm() const { return _alarm; }
  uint8_t alarmCode() const { return _alarmCode; }

private:
  TempControl _temp;
  HumidityControl _humCtl;
  MotorSchedule _motorSch;
  VentilationTimer _vent;

  PersistedData _cfg{};
  EffectiveTargets _targets{};

  bool _heater = false;
  bool _motor = false;
  bool _fan = false;
  bool _humid = false;

  // alarms
  bool _alarm = false;
  uint8_t _alarmCode = 0; // 1=sensor, 2=overtemp

  uint8_t _sensorFailCount = 0;

  uint32_t _bootMs = 0;
  int16_t _currentTemp_x10 = 0;
  int16_t _currentHum_x10  = 0;

  static int16_t toX10(float v);
  void computeTargets();
  void clearAlarmIfSafe();
};

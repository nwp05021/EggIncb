#include "IncubatorController.h"
#include "../../config/Defaults.h"

int16_t IncubatorController::toX10(float v) {
  int x = (int)(v * 10.0f + (v >= 0 ? 0.5f : -0.5f));
  if (x > 32767) x = 32767;
  if (x < -32768) x = -32768;
  return (int16_t)x;
}

void IncubatorController::begin(const PersistedData& cfg, uint32_t bootMs) {
  _bootMs = bootMs;
  // ventilation defaults: 30 min / 2 min
  _vent.configure(30UL * 60UL * 1000UL, 2UL * 60UL * 1000UL);
  applyConfig(cfg);
}

void IncubatorController::computeTargets() {
  uint8_t day = _cfg.incubationDay;
  if (day < 1) day = 1;
  if (day > 21) day = 21;

  if (_cfg.scheduleMode == 0) {
    auto t = IncubationSchedule::targetsForDay(day);
    _targets.temp_x10 = t.temp_x10;
    _targets.hum_x10  = t.hum_x10;
  } else {
    _targets.temp_x10 = _cfg.targetTemp_x10;
    _targets.hum_x10  = _cfg.targetHum_x10;
  }
}

void IncubatorController::applyConfig(const PersistedData& cfg) {
  _cfg = cfg;

  if (_cfg.humHyst_x10 <= 0) _cfg.humHyst_x10 = DEFAULT_HUM_HYST_X10;
  if (_cfg.tempHyst_x10 <= 0) _cfg.tempHyst_x10 = DEFAULT_TEMP_HYST_X10;
  if (_cfg.incubationDay < 1) _cfg.incubationDay = DEFAULT_INCUBATION_DAY;
  if (_cfg.incubationDay > 21) _cfg.incubationDay = 21;
  if (_cfg.scheduleMode > 1) _cfg.scheduleMode = DEFAULT_SCHEDULE_MODE;

  computeTargets();

  _temp.configure(_targets.temp_x10, _cfg.tempHyst_x10);
  _humCtl.configure(_targets.hum_x10, _cfg.humHyst_x10);
  _motorSch.configure(_cfg.motorOnSec, _cfg.motorOffMin);
}

void IncubatorController::clearAlarmIfSafe() {
  // keep alarm latched until conditions are safe for a while (simple)
  if (_alarmCode == 2) { // overtemp
    if (_currentTemp_x10 < (_targets.temp_x10 + 20)) { // 2.0C below target
      _alarm = false;
      _alarmCode = 0;
    }
  }
  if (_alarmCode == 1) { // sensor
    if (_sensorFailCount == 0) {
      _alarm = false;
      _alarmCode = 0;
    }
  }
}

void IncubatorController::update(float currentTempC, float currentHumRH, bool sensorOk, uint32_t nowMs) {
  _currentTemp_x10 = toX10(currentTempC);
  _currentHum_x10  = toX10(currentHumRH);

  // recompute targets if day/mode changed (cheap)
  computeTargets();
  _temp.configure(_targets.temp_x10, _cfg.tempHyst_x10);
  _humCtl.configure(_targets.hum_x10, _cfg.humHyst_x10);

  // sensor health
  if (!sensorOk) {
    if (_sensorFailCount < 255) _sensorFailCount++;
  } else {
    _sensorFailCount = 0;
  }

  // ventilation timer
  _vent.update(nowMs);

  // motor schedule
  _motorSch.update(nowMs);
  _motor = (_cfg.motorEnabled != 0) ? _motorSch.motorOn() : false;

  // heater safety
  bool heaterAllowed = (nowMs - _bootMs) >= HEATER_LOCKOUT_MS;
  bool overTemp = _currentTemp_x10 >= SAFETY_MAX_TEMP_X10;

  if (!sensorOk || overTemp || !heaterAllowed || (_cfg.heaterEnabled == 0)) {
    _heater = false;
  } else {
    _temp.update(_currentTemp_x10);
    _heater = _temp.heaterOn();
  }

  // humidifier safety
  if (!sensorOk || (_cfg.humidifierEnabled == 0)) {
    _humid = false;
  } else {
    _humCtl.update(_currentHum_x10);
    _humid = _humCtl.humidifierOn();
  }

  // alarms (latched)
  if (_sensorFailCount >= 3) {
    _alarm = true;
    _alarmCode = 1;
    _heater = false;
    _humid = false;
  }
  if (overTemp) {
    _alarm = true;
    _alarmCode = 2;
    _heater = false;
  }
  clearAlarmIfSafe();

  // fan policy: enabled AND (heater OR humidifier OR ventilation active)
  bool ventOn = _vent.active();
  _fan = (_cfg.fanEnabled != 0) ? (_heater || _humid || ventOn) : false;
}

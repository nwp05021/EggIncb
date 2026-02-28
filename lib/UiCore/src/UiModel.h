#pragma once
#include <stdint.h>

struct UiModel {
  // live
  int16_t currentTemp_x10 = 0;
  int16_t currentHum_x10  = 0; // RH x10
  bool sensorOk = false;

  bool heaterOn = false;
  bool motorOn = false;
  bool fanOn = false;
  bool humidifierOn = false;

  bool alarm = false;
  uint8_t alarmCode = 0;

  bool wifiConnected = false;
  bool provisioning = false;

  // effective targets (AUTO/MANUAL resolved in controller)
  int16_t effectiveTargetTemp_x10 = 375;
  int16_t effectiveTargetHum_x10  = 550;

  // schedule
  uint8_t presetId = 0;

  // elapsedDay is derived from (start date + current time) and is used for display and schedule.
  // 0 means "unknown" (time not synced yet).
  uint8_t elapsedDay = 0;

  // incubation start date (for elapsedDay)
  uint16_t startYear = 0;
  uint8_t  startMonth = 0;
  uint8_t  startDay = 0;

  int16_t tempHyst_x10   = 3;
  int16_t humHyst_x10    = 30;

  uint16_t motorOnSec = 10;
  uint16_t motorOffMin = 30;

  bool heaterEnabled = true;
  bool motorEnabled  = true;
  bool fanEnabled    = true;
  bool humidifierEnabled = false;

  // UI runtime
  uint8_t mainPage = 0;       // rotate on main
  bool blink = false;

  char timeStr[9];   // "HH:MM:SS"
};

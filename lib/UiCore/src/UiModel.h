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

  // config mirror (manual targets + schedule)
  uint8_t scheduleMode = 0;      // 0=AUTO, 1=MANUAL
  uint8_t incubationDay = 1;     // 1..21

  uint16_t startYear = 0;
  uint8_t  startMonth = 0;
  uint8_t  startDay = 0;

  int16_t targetTemp_x10 = 375;
  int16_t tempHyst_x10   = 3;

  int16_t targetHum_x10  = 550;
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
};

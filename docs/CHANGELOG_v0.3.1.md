# IncubatorC3 Framework v0.3.1

## Fixed
- UI MainPage render: drawMain signature now passes uptime (millis).

## Added (Domain)
- HumidityControl (hysteresis ON/OFF control for humidifier relay)
- IncubationSchedule (day-based default targets: D1~18, D19~21)
- AUTO/MANUAL schedule mode:
  - AUTO: effective targets are derived from day schedule
  - MANUAL: use user targets

## Updated
- PersistedData v4:
  - Added scheduleMode, incubationDay, humHyst_x10, renamed hysteresis -> tempHyst_x10
- Settings menu:
  - schedule mode, incubation day, target humidity, humidity hysteresis, fan/humid enable
- Main UI:
  - P1: current temp/hum
  - P2: effective targets (AUTO aware)
  - P3: motor summary + heater/humid status

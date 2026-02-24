# IncubatorC3 Framework v0.5.0

## Added
- Calendar-based incubation start date input (YYYY-MM-DD) via Settings menu.
- Start date fields persisted: startYear/startMonth/startDay.
- incubationStartEpoch is derived from the start date (local midnight) when time is valid.
- NTP sync (KST): after Wi-Fi connects, configTime() runs once and TimeManager is upgraded to real epoch.
- AUTO mode day is computed from real epoch and start date.

## Fixed
- UiRenderer indicator insertion: removed any drawHeader dependency on UiModel, cleaned escaped quotes, standardized u8g2 usage.
- Provisioning stop: removed core/IDF stop calls for maximum Arduino-ESP32 compatibility.

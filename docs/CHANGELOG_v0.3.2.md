# IncubatorC3 Framework v0.3.2

## Added
- TimeManager: monotonic epoch seconds with periodic persistence for power-loss recovery
- AUTO mode day auto-increment based on incubationStartEpoch
- PersistedData v5: incubationStartEpoch, lastKnownEpoch
- Alarm system (latched):
  - code 1: sensor fail (>=3 consecutive)
  - code 2: overtemp
- Fan ventilation timer: 30 min interval, 2 min duration (fan enabled + OR-ed into fan policy)
- UI: alarm indicator ("ALM") on header

## Notes
- This version uses a monotonic epoch counter (seconds) even without RTC/NTP.
- Later: replace TimeManager epoch source with RTC/NTP for real-world time.

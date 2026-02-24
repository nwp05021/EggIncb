# IncubatorC3 Framework v0.6.0

## UI Stability
- EditValuePage is now type-safe: supports U8/U16/I16_X10/Bool without pointer aliasing.
- Fixed incorrect values when editing byte-sized fields (e.g., start month/day showing huge numbers).

## Settings UX
- Settings menu labels are compact ASCII (OLED-friendly): SCHED/DAY/SY/SM/SD/TEMP/HYST/...
- Edit labels/units are also ASCII to avoid large Korean font in tight menu layouts.

## Calendar-based Incubation
- Auto day calculation is corrected and gated by `scheduleMode==AUTO` and valid epoch time.
- Start date (Y/M/D) is converted to local-midnight epoch and persisted.

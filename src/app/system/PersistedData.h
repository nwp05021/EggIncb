#pragma once
#include <stdint.h>

#pragma pack(push, 1)
struct PersistedData {
  uint32_t magic;
  uint16_t version;

  // Manual targets (x10)
  int16_t  targetTemp_x10;
  int16_t  tempHyst_x10;

  int16_t  targetHum_x10;
  int16_t  humHyst_x10;

  // Schedule
  uint8_t  scheduleMode;    // 0=AUTO, 1=MANUAL
  uint8_t  incubationDay;   // 1..21 (AUTO updates)

  // Actuators
  uint16_t motorOnSec;
  uint16_t motorOffMin;

  uint8_t  heaterEnabled;
  uint8_t  motorEnabled;
  uint8_t  fanEnabled;
  uint8_t  humidifierEnabled;

  // Date-based incubation start (local date)
  uint16_t startYear;   // e.g. 2026
  uint8_t  startMonth;  // 1..12
  uint8_t  startDay;    // 1..31

  // Timekeeping (seconds)
  uint32_t incubationStartEpoch; // derived from YYYY-MM-DD when time is valid (0 => unset)
  uint32_t lastKnownEpoch;       // persisted epoch for power-loss recovery (0 => unknown)

  uint32_t resetCount;

  uint32_t crc; // CRC32 of all bytes except this field
};
#pragma pack(pop)

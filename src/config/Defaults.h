#pragma once
#include <stdint.h>

// Temperature is stored as x10 (37.5°C => 375)
// NOTE: per-day targets are stored in the schedule table. No global target value.
static constexpr int16_t DEFAULT_TEMP_HYST_X10   = 3;   // 0.3°C

// Humidity is stored as x10 (55.0% => 550)
// NOTE: per-day targets are stored in the schedule table. No global target value.
static constexpr int16_t DEFAULT_HUM_HYST_X10    = 30;  // 3.0%

static constexpr uint16_t DEFAULT_MOTOR_ON_SEC   = 10;
static constexpr uint16_t DEFAULT_MOTOR_OFF_MIN  = 30;

static constexpr uint8_t DEFAULT_HEATER_ENABLED  = 1;
static constexpr uint8_t DEFAULT_MOTOR_ENABLED   = 1;
static constexpr uint8_t DEFAULT_FAN_ENABLED     = 1;
static constexpr uint8_t DEFAULT_HUMID_ENABLED   = 0;

// Day is derived from (start date + current time). UI does not directly edit the day.

// Default incubation start date (for elapsed-day calculation)
// NOTE: This is only an initial value. User can change it in the settings menu.
static constexpr uint16_t DEFAULT_START_YEAR  = 2026;
static constexpr uint8_t  DEFAULT_START_MONTH = 1;
static constexpr uint8_t  DEFAULT_START_DAY   = 1;

// Preset IDs (for editable per-day schedule table)
static constexpr uint8_t PRESET_CHICKEN_STD   = 0;
static constexpr uint8_t PRESET_CHICKEN_HATCH = 1;
static constexpr uint8_t PRESET_CLEAR_CUSTOM  = 2;
static constexpr uint8_t PRESET_COUNT         = 3;

// Safety
static constexpr int16_t SAFETY_MAX_TEMP_X10     = 450;     // 45.0°C => force heater OFF
static constexpr uint32_t HEATER_LOCKOUT_MS      = 10000;  // boot lockout

// Persisted settings
// v8: removed scheduleMode/incubationDay/manual targets. Day is derived from start date.
static constexpr uint16_t SETTINGS_VERSION = 8;
static constexpr uint32_t SETTINGS_MAGIC   = 0x1AC0B0A7;

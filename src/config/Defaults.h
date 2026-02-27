#pragma once
#include <stdint.h>

// Temperature is stored as x10 (37.5°C => 375)
static constexpr int16_t DEFAULT_TARGET_TEMP_X10 = 375;
static constexpr int16_t DEFAULT_TEMP_HYST_X10   = 3;   // 0.3°C

// Humidity is stored as x10 (55.0% => 550)
static constexpr int16_t DEFAULT_TARGET_HUM_X10  = 550;
static constexpr int16_t DEFAULT_HUM_HYST_X10    = 30;  // 3.0%

static constexpr uint16_t DEFAULT_MOTOR_ON_SEC   = 10;
static constexpr uint16_t DEFAULT_MOTOR_OFF_MIN  = 30;

static constexpr uint8_t DEFAULT_HEATER_ENABLED  = 1;
static constexpr uint8_t DEFAULT_MOTOR_ENABLED   = 1;
static constexpr uint8_t DEFAULT_FAN_ENABLED     = 1;
static constexpr uint8_t DEFAULT_HUMID_ENABLED   = 0;

// Schedule
// 0=AUTO (Day schedule), 1=MANUAL (use target values)
static constexpr uint8_t DEFAULT_SCHEDULE_MODE   = 0;
static constexpr uint8_t DEFAULT_INCUBATION_DAY  = 1;

// Start date defaults (used for elapsed-day calculation)
// NOTE: This is only an initial value. User can change it in the settings menu.
static constexpr uint16_t DEFAULT_START_YEAR  = 2026;
static constexpr uint8_t  DEFAULT_START_MONTH = 1;
static constexpr uint8_t  DEFAULT_START_DAY   = 1;

// Safety
static constexpr int16_t SAFETY_MAX_TEMP_X10     = 450;     // 45.0°C => force heater OFF
static constexpr uint32_t HEATER_LOCKOUT_MS      = 10000;  // boot lockout

// Persisted settings
static constexpr uint16_t SETTINGS_VERSION = 6;
static constexpr uint32_t SETTINGS_MAGIC   = 0x1AC0B0A7;

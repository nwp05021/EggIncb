#include <Arduino.h>

#include "config/PinMap.h"
#include "config/Defaults.h"

#include "hal/I2cHal.h"
#include "hal/RelayHal.h"
#include "hal/I2cLock.h"

#include "app/system/PersistedData.h"
#include "app/system/SettingsStore.h"

#include "app/services/EnvSensor_AHT.h"
#include "app/controllers/IncubatorController.h"

// lib/InputCore
#include <EncoderHal_ArduinoEsp32.h>
#include <EncoderController.h>
#include <EncoderAccel.h>

// lib/UiCore
#include <UiApp.h>
#include <UiRenderer.h> 

#include <esp_task_wdt.h>

// static 붙이면 안 된다.
I2cLock g_i2cLock;

static SettingsStore settings;
static PersistedData cfg;

// cfg mirror for UI (avoid bool/uint8 aliasing)
static bool cfgHeaterEnabled = true;
static bool cfgMotorEnabled  = true;
static bool cfgFanEnabled    = true;
static bool cfgHumidEnabled  = false;

static EnvSensor_AHT env;
static IncubatorController incubator;
static RelayHal relay;

static EncoderHal_ArduinoEsp32 encHal(PIN_ENCODER_A, PIN_ENCODER_B, PIN_ENCODER_BTN);
static EncoderController encoder(encHal);
static EncoderAccel accel;

static UiRenderer uiRenderer;
static UiApp ui;

static volatile bool g_persistDirty = false;
static uint32_t g_persistDirtyAt = 0;

//---------------------------------------------
// Improved Persist Policy
//---------------------------------------------
static uint32_t g_lastInputMs = 0;
static uint32_t g_lastSaveMs = 0;

static void setDefaults(PersistedData& d) {
  memset(&d, 0, sizeof(d));
  d.magic = SETTINGS_MAGIC;
  d.version = SETTINGS_VERSION;

  d.scheduleMode = DEFAULT_SCHEDULE_MODE;
  d.incubationDay = DEFAULT_INCUBATION_DAY;

  d.targetTemp_x10 = DEFAULT_TARGET_TEMP_X10;
  d.tempHyst_x10   = DEFAULT_TEMP_HYST_X10;

  d.targetHum_x10  = DEFAULT_TARGET_HUM_X10;
  d.humHyst_x10    = DEFAULT_HUM_HYST_X10;

  d.motorOnSec  = DEFAULT_MOTOR_ON_SEC;
  d.motorOffMin = DEFAULT_MOTOR_OFF_MIN;

  d.heaterEnabled     = DEFAULT_HEATER_ENABLED;
  d.motorEnabled      = DEFAULT_MOTOR_ENABLED;
  d.fanEnabled        = DEFAULT_FAN_ENABLED;
  d.humidifierEnabled = DEFAULT_HUMID_ENABLED;

  d.resetCount = 0;
  d.crc = 0;
}

void markPersistDirty() {
  g_persistDirty = true;
  g_persistDirtyAt = millis();
}

// Called from UI confirm page (linked symbol)
void ui_factory_reset() {
  Serial.println("[UI] factory reset requested");
  ui.requestFactoryReset();
}

static void syncCfgToUiMirror() {
  cfgHeaterEnabled = (cfg.heaterEnabled != 0);
  cfgMotorEnabled  = (cfg.motorEnabled != 0);
  cfgFanEnabled    = (cfg.fanEnabled != 0);
  cfgHumidEnabled  = (cfg.humidifierEnabled != 0);
}

static void syncUiMirrorToCfg() {
  cfg.heaterEnabled     = cfgHeaterEnabled ? 1 : 0;
  cfg.motorEnabled      = cfgMotorEnabled ? 1 : 0;
  cfg.fanEnabled        = cfgFanEnabled ? 1 : 0;
  cfg.humidifierEnabled = cfgHumidEnabled ? 1 : 0;
}

static void applyCfgToUiModel() {
  UiModel& m = ui.model();
  m.scheduleMode   = cfg.scheduleMode;
  m.incubationDay  = cfg.incubationDay;
  m.targetTemp_x10 = cfg.targetTemp_x10;
  m.tempHyst_x10   = cfg.tempHyst_x10;
m.targetHum_x10  = cfg.targetHum_x10;
  m.motorOnSec = cfg.motorOnSec;
  m.motorOffMin = cfg.motorOffMin;
  m.heaterEnabled = cfgHeaterEnabled;
  m.motorEnabled = cfgMotorEnabled;
  m.fanEnabled   = cfgFanEnabled;
  m.humidifierEnabled = cfgHumidEnabled;
  m.fanEnabled = (cfg.fanEnabled != 0);
  m.humidifierEnabled = (cfg.humidifierEnabled != 0);
}

static void applyUiModelToCfg() {
  UiModel& m = ui.model();
  cfg.scheduleMode  = m.scheduleMode;
  cfg.incubationDay = m.incubationDay;
  cfg.targetTemp_x10 = m.targetTemp_x10;
  cfg.tempHyst_x10 = m.tempHyst_x10;
cfg.motorOnSec = m.motorOnSec;
  cfg.motorOffMin = m.motorOffMin;
  cfgHeaterEnabled = m.heaterEnabled;
  cfgMotorEnabled  = m.motorEnabled;
  cfgFanEnabled    = m.fanEnabled;
  cfgHumidEnabled  = m.humidifierEnabled;
  syncUiMirrorToCfg();
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("[IncubatorC3] boot v0.3.0");

  settings.begin("incubator");

  if (!settings.load(cfg) || cfg.magic != SETTINGS_MAGIC || cfg.version != SETTINGS_VERSION) {
    Serial.println("[Settings] load failed -> defaults");
    setDefaults(cfg);
    settings.save(cfg);
  }

  cfg.resetCount++;
  settings.save(cfg);

  syncCfgToUiMirror();

  I2cHal::begin(PIN_I2C_SDA, PIN_I2C_SCL);
  relay.begin(PIN_HEATER_RELAY, PIN_MOTOR_RELAY, PIN_FAN_RELAY, PIN_HUMID_RELAY, true);

  bool sensorOk = env.begin();
  Serial.printf("[AHT] %s\n", sensorOk ? "OK" : "FAIL");

  incubator.begin(cfg, millis());

  // input
  EncoderConfig encCfg;
  encoder.begin(encCfg);

  EncoderAccelConfig accCfg;
  accel.begin(accCfg);

  // UI
  UiCallbacks cb;
  cb.onConfigChanged = []() {
    applyUiModelToCfg();
    markPersistDirty();
  };

  ui.begin(&uiRenderer, cb);
  ui.bindConfig(&cfg.scheduleMode,
               &cfg.incubationDay,
               &cfg.startYear,
               &cfg.startMonth,
               &cfg.startDay,
               &cfg.targetTemp_x10,
               &cfg.tempHyst_x10,
               &cfg.targetHum_x10,
               &cfg.humHyst_x10,
               &cfg.motorOnSec,
               &cfg.motorOffMin,
               &cfgHeaterEnabled,
               &cfgMotorEnabled,
               &cfgFanEnabled,
               &cfgHumidEnabled);

  applyCfgToUiModel();

  g_i2cLock.begin();
  
  //---------------------------------------------
  // WDT: 5초 안에 loop가 돌지 않으면 리셋
  //---------------------------------------------
  esp_task_wdt_init(5, true);  // timeout=5s, panic+reset
  esp_task_wdt_add(NULL);      // 현재 task(loop task) 등록

  //---------------------------------------------
  // Improved Persist Policy
  //---------------------------------------------
  g_lastSaveMs = millis();
}

void loop() {
  uint32_t now = millis();

  static uint32_t s_lastLog = 0;
  static uint32_t s_lastNow = 0;

  uint32_t dt = (s_lastNow == 0) ? 0 : (now - s_lastNow);
  s_lastNow = now;

  if (now - s_lastLog > 5000) {
    s_lastLog = now;
    Serial.printf("[Loop] dt=%lu ms heap=%u\n",
      (unsigned long)dt, (unsigned)ESP.getFreeHeap());
  }  

  // encoder events
  EncoderEvents e = encoder.poll();
  e = accel.apply(e, now);

  //---------------------------------------------
  // Improved Persist Policy
  //---------------------------------------------
  // if (e.delta != 0) ui.onEncoder(e.delta);
  // if (e.shortPress) ui.onClick();
  // if (e.longPress) ui.onLongPress();
  // if (e.veryLongPress) ui.onVeryLongPress();
  if (e.delta != 0) {
    ui.onEncoder(e.delta);
    g_lastInputMs = now;
  }
  if (e.shortPress) {
    ui.onClick();
    g_lastInputMs = now;
  }
  if (e.longPress) {
    ui.onLongPress();
    g_lastInputMs = now;
  }
  if (e.veryLongPress) {
    ui.onVeryLongPress();
    g_lastInputMs = now;
  }

  // sensor
  env.tick(now);

  // control
  incubator.applyConfig(cfg);
  incubator.update(env.temperatureC(), env.humidityRH(), env.ok(), now);

  // Fan policy (simple): on if enabled
  bool fanOn = (cfg.fanEnabled != 0);
  bool humidOn = false; // reserved (later humidity controller)

  relay.setHeater(incubator.heaterState());
  relay.setMotor(incubator.motorState());
  relay.setFan(fanOn);
  relay.setHumidifier(humidOn);

  // update model live
  UiModel& m = ui.model();
  m.currentTemp_x10 = incubator.currentTemp_x10();
  m.currentHum_x10  = incubator.currentHum_x10();
  m.currentHum_x10 = (int16_t)(env.humidityRH() * 10.0f + 0.5f);
  m.sensorOk = env.ok();

  m.heaterOn = relay.heater();
  m.motorOn  = relay.motor();
  m.fanOn    = relay.fan();
  m.humidifierOn = relay.humidifier();

  // effective targets (AUTO/MANUAL resolved)
  auto t = incubator.targets();
  m.effectiveTargetTemp_x10 = t.temp_x10;
  m.effectiveTargetHum_x10  = t.hum_x10;


  // optional: day counter placeholder (can be derived from RTC later)
  // m.incubationDay = ...

  ui.tick(now);
  ui.render();

  // handle factory reset request from UI
  if (ui.takeFactoryResetRequest()) {
    settings.factoryReset();
    setDefaults(cfg);
    settings.save(cfg);
    syncCfgToUiMirror();
    applyCfgToUiModel();
    markPersistDirty();
    Serial.println("[Settings] factory reset done");
  }

  //---------------------------------------------
  // Improved Persist Policy
  //---------------------------------------------
  // debounced persist: save 1s after last change
  // if (g_persistDirty && (now - g_persistDirtyAt) > 1000) {
  //   g_persistDirty = false;
  //   cfg.magic = SETTINGS_MAGIC;
  //   cfg.version = SETTINGS_VERSION;
  //   settings.save(cfg);
  //   Serial.println("[Settings] saved");
  // }
  if (g_persistDirty &&
      (now - g_persistDirtyAt) > 15000 &&   // 최소 15초 대기
      (now - g_lastInputMs) > 3000 &&      // 최근 3초간 입력 없음
      (now - g_lastSaveMs) > 30000)        // 최소 저장 간격 30초
  {
      g_persistDirty = false;
      g_lastSaveMs = now;

      cfg.magic = SETTINGS_MAGIC;
      cfg.version = SETTINGS_VERSION;

      settings.save(cfg);
      Serial.println("[Settings] saved (delayed)");
  }  

  delay(10);

  // WDT
  esp_task_wdt_reset();
}

#include "IncubatorApp.h"

#include <EncoderHal_ArduinoEsp32.h>
#include <EncoderController.h>
#include <EncoderAccel.h>

#include "config/PinMap.h"
#include "config/Defaults.h"
#include "hal/I2cHal.h"

#include "hal/I2cLock.h"

extern I2cLock g_i2cLock;

void IncubatorApp::syncCfgToBools() {
  _heaterEn = (_cfg.heaterEnabled != 0);
  _motorEn  = (_cfg.motorEnabled  != 0);
  _fanEn    = (_cfg.fanEnabled    != 0);
  _humidEn  = (_cfg.humidifierEnabled != 0);
}

void IncubatorApp::syncBoolsToCfg() {
  _cfg.heaterEnabled     = _heaterEn ? 1 : 0;
  _cfg.motorEnabled      = _motorEn  ? 1 : 0;
  _cfg.fanEnabled        = _fanEn    ? 1 : 0;
  _cfg.humidifierEnabled = _humidEn  ? 1 : 0;
}

static IncubatorApp* s_app = nullptr;

static void ui_on_config_changed() {
  if (!s_app) return;
  s_app->syncBoolsToCfg();
  s_app->markPersistDirty();
}

void IncubatorApp::markPersistDirty() {
  _persistDirty = true;
  _persistDirtyAt = millis();
}

static EncoderHal_ArduinoEsp32 encHal(PIN_ENCODER_A,
                                      PIN_ENCODER_B,
                                      PIN_ENCODER_BTN);
static EncoderController encoder(encHal);
static EncoderAccel accel;

static void applyPresetTable(PersistedData& d, uint8_t presetId) {
  // NOTE: values are x10.
  // The table is always editable from UI; presets are just "starting points".
  switch (presetId) {
    case PRESET_CHICKEN_STD:
      // Day 1-21: 37.5C / 55%
      for (int i = 0; i < 21; ++i) {
        d.dayTemp_x10[i] = 375;
        d.dayHum_x10[i]  = 550;
      }
      break;

    case PRESET_CHICKEN_HATCH:
      // Day 1-18: 37.5C / 55%
      // Day 19-21: 37.2C / 70%
      for (int i = 0; i < 21; ++i) {
        if (i < 18) {
          d.dayTemp_x10[i] = 375;
          d.dayHum_x10[i]  = 550;
        } else {
          d.dayTemp_x10[i] = 372;
          d.dayHum_x10[i]  = 700;
        }
      }
      break;

    case PRESET_CLEAR_CUSTOM:
    default:
      for (int i = 0; i < 21; ++i) {
        d.dayTemp_x10[i] = 0;
        d.dayHum_x10[i]  = 0;
      }
      break;
  }
}

static void setDefaults(PersistedData& d) {
  memset(&d, 0, sizeof(d));
  d.magic = SETTINGS_MAGIC;
  d.version = SETTINGS_VERSION;

  d.presetId     = PRESET_CHICKEN_HATCH;

  d.startYear  = DEFAULT_START_YEAR;
  d.startMonth = DEFAULT_START_MONTH;
  d.startDay   = DEFAULT_START_DAY;

  d.tempHyst_x10   = DEFAULT_TEMP_HYST_X10;
  d.humHyst_x10    = DEFAULT_HUM_HYST_X10;

  d.motorOnSec  = DEFAULT_MOTOR_ON_SEC;
  d.motorOffMin = DEFAULT_MOTOR_OFF_MIN;

  d.heaterEnabled     = DEFAULT_HEATER_ENABLED;
  d.motorEnabled      = DEFAULT_MOTOR_ENABLED;
  d.fanEnabled        = DEFAULT_FAN_ENABLED;
  d.humidifierEnabled = DEFAULT_HUMID_ENABLED;

  applyPresetTable(d, d.presetId);
}

void IncubatorApp::begin()
{
  s_app = this;

  Serial.begin(115200);
  delay(200);

  _settings.begin("incubator");
  if (!_settings.load(_cfg) || _cfg.magic != SETTINGS_MAGIC || _cfg.version != SETTINGS_VERSION) {
    Serial.println("[Settings] load failed -> defaults");
    setDefaults(_cfg);
    _settings.save(_cfg);
  }

  // If schedule table is empty (e.g. upgraded struct), seed defaults once.
  if (_cfg.dayTemp_x10[0] == 0) {
    if (_cfg.presetId >= PRESET_COUNT) _cfg.presetId = PRESET_CHICKEN_HATCH;
    applyPresetTable(_cfg, _cfg.presetId);
    _settings.save(_cfg);
  }

  syncCfgToBools();  // ✅ 중요: bindConfig 전에

  I2cHal::begin(PIN_I2C_SDA, PIN_I2C_SCL);
  g_i2cLock.begin();

  EncoderConfig encCfg;
  encoder.begin(encCfg);

  EncoderAccelConfig accCfg;
  accel.begin(accCfg);

  _relay.begin(PIN_HEATER_RELAY, PIN_MOTOR_RELAY, PIN_FAN_RELAY, PIN_HUMID_RELAY, true);

  _env.begin();

  UiCallbacks cb;
  cb.onConfigChanged = ui_on_config_changed;
  _ui.begin(&_renderer, cb);

  // Settings: start date + hysteresis + motor timing + enables.
  _ui.bindConfig(&_cfg.startYear,
                &_cfg.startMonth,
                &_cfg.startDay,
                &_cfg.presetId,
                _cfg.dayTemp_x10,
                _cfg.dayHum_x10,
                &_cfg.tempHyst_x10,
                &_cfg.humHyst_x10,
                &_cfg.motorOnSec,
                &_cfg.motorOffMin,
                &_heaterEn, &_motorEn, &_fanEn, &_humidEn);  // ✅ nullptr 금지

  _prov.begin();
  _timeMgr.begin();

  _lastSaveMs = millis();
}

void IncubatorApp::tick()
{
    static uint32_t lastInputMs = 0;
    const uint32_t INPUT_DEBOUNCE_MS = 3;

    uint32_t now = millis();

    // 1️⃣ 입력 처리
    EncoderEvents e = encoder.poll();
    e = accel.apply(e, now);

    if (now - lastInputMs > INPUT_DEBOUNCE_MS)
    {
        if (e.delta != 0) {
            _ui.onEncoder(e.delta);
            lastInputMs = now;
            _lastInputMs = now;
        }

        if (e.longPress) {
            _ui.onLongPress();
            lastInputMs = now;
            _lastInputMs = now;
        }
        else if (e.shortPress) {
            _ui.onClick();
            lastInputMs = now;
            _lastInputMs = now;
        }
        else if (e.veryLongPress) {
            _ui.onVeryLongPress();
            lastInputMs = now;
            _lastInputMs = now;
        }
    }    

    // 2️⃣ 서비스 업데이트
    _prov.tick(now);
    _timeMgr.loop();
    _env.tick(now);

    // 3️⃣ 경과일, 제어 계산
    computeElapsedDay();
    applyRuntimeDay();

    _incubator.applyConfig(_cfg);
    _incubator.update(_env.temperatureC(),
                     _env.humidityRH(),
                     _env.ok(),
                     now);

    _relay.setHeater(_incubator.heaterState());
    _relay.setMotor(_incubator.motorState());
    _relay.setFan(_incubator.fanState());
    _relay.setHumidifier(_incubator.humidifierState());

    // 4️⃣ UI 모델 갱신 (🔥 반드시 계산 후에)
    UiModel& m = _ui.model();

    m.currentTemp_x10 = _incubator.currentTemp_x10();
    m.currentHum_x10  = _incubator.currentHum_x10();
    m.sensorOk = _env.ok();

    m.heaterOn = _incubator.heaterState();
    m.motorOn  = _incubator.motorState();
    m.fanOn    = _incubator.fanState();
    m.humidifierOn = _incubator.humidifierState();

    m.wifiConnected = _prov.isWifiConnected();
    m.provisioning  = _prov.isProvisioning();

    // ✅ Today targets (Table-only control)
    {
      auto tgt = _incubator.targets();
      m.effectiveTargetTemp_x10 = tgt.temp_x10;
      m.effectiveTargetHum_x10  = tgt.hum_x10;
    }

    _timeMgr.getTimeString(m.timeStr, sizeof(m.timeStr));

    // 5️⃣ 렌더
    _ui.tick(now);
    _ui.render();

    // 6️⃣ 저장 (TABLE 포함) - 사용자 입력 멈춘 뒤 짧게 지연 저장
    if (_persistDirty &&
        (now - _persistDirtyAt) > 1200 &&
        (now - _lastInputMs) > 600 &&
        (now - _lastSaveMs) > 1200)
    {
        _persistDirty = false;
        _lastSaveMs = now;
        _cfg.magic = SETTINGS_MAGIC;
        _cfg.version = SETTINGS_VERSION;
        _settings.save(_cfg);
        Serial.println("[Settings] saved");
    }

    if (_factoryResetRequested)
    {
        _settings.factoryReset();
        ESP.restart();
    }

    // ✅ 치명적 Fault 시 Safe Mode
    if (_incubator.alarmCode() == 2) // overtemp
    {
        _relay.setHeater(false);
        _relay.setFan(true);
    }    
}

//-----------------------------------------------
// 경과일 계산 (AUTO 스케줄과 분리)
//-----------------------------------------------
void IncubatorApp::computeElapsedDay()
{
    time_t nowEpoch = _timeMgr.now();
    if (nowEpoch == 0) return;

    struct tm startTm = {};
    startTm.tm_year = _cfg.startYear - 1900;
    startTm.tm_mon  = _cfg.startMonth - 1;
    startTm.tm_mday = _cfg.startDay;

    time_t startEpoch = mktime(&startTm);
    if (startEpoch <= 0) return;

    int days = (nowEpoch - startEpoch) / 86400;
    if (days < 0) days = 0;
    if (days > 21) days = 21;

    // 🔥 표시용은 UiModel로 전달
    _ui.model().elapsedDay = days + 1;
}

//-----------------------------------------------
// AUTO 모드에서 runtime day 적용
//-----------------------------------------------
void IncubatorApp::applyRuntimeDay()
{
    // Day is derived from start date + current time.
    // If time isn't synced yet, stay at Day 1.
    uint8_t day = _ui.model().elapsedDay;
    if (day < 1) day = 1;
    if (day > 21) day = 21;

    _incubator.setRuntimeDay(day);
}

//-----------------------------------------------
// UI 브릿지 함수 구현
//-----------------------------------------------
void IncubatorApp::requestFactoryReset()
{
    _factoryResetRequested = true;
}

void IncubatorApp::requestProvisioningReset()
{
    _prov.resetProvisioning();
}

void IncubatorApp::requestTimeSync()
{
    _timeMgr.requestSync();
}
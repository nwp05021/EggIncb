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
  // 필요하면 settings save dirty 처리도 여기서
}

static EncoderHal_ArduinoEsp32 encHal(PIN_ENCODER_A,
                                      PIN_ENCODER_B,
                                      PIN_ENCODER_BTN);
static EncoderController encoder(encHal);
static EncoderAccel accel;

void IncubatorApp::begin()
{
  s_app = this;

  Serial.begin(115200);
  delay(200);

  _settings.begin("incubator");
  if (!_settings.load(_cfg) || _cfg.magic != SETTINGS_MAGIC || _cfg.version != SETTINGS_VERSION) {
    memset(&_cfg, 0, sizeof(_cfg));
    _cfg.magic = SETTINGS_MAGIC;
    _cfg.version = SETTINGS_VERSION;
    // TODO: 여기서 defaults 세팅 (startYear/Month/Day 포함)
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

  _ui.bindConfig(&_cfg.scheduleMode,
                &_cfg.startYear,
                &_cfg.startMonth,
                &_cfg.startDay,
                &_cfg.targetTemp_x10,
                &_cfg.tempHyst_x10,
                &_cfg.targetHum_x10,
                &_cfg.humHyst_x10,
                &_cfg.motorOnSec,
                &_cfg.motorOffMin,
                &_heaterEn, &_motorEn, &_fanEn, &_humidEn);  // ✅ nullptr 금지

  _prov.begin();
  _timeMgr.begin();
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
        }

        if (e.longPress) {
            _ui.onLongPress();
            lastInputMs = now;
        }
        else if (e.shortPress) {
            _ui.onClick();
            lastInputMs = now;
        }
        else if (e.veryLongPress) {
            _ui.onVeryLongPress();
            lastInputMs = now;
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

    _timeMgr.getTimeString(m.timeStr, sizeof(m.timeStr));

    // 5️⃣ 렌더
    _ui.tick(now);
    _ui.render();

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
    if (_cfg.scheduleMode == 0)
    {
        _incubator.setRuntimeDay(_ui.model().elapsedDay);
    }
    else
    {
        _incubator.setRuntimeDay(1); // MANUAL 모드에서는 의미 없음
    }
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
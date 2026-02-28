#include "UiApp.h"
#include "pages/IncubatorMainPage.h"
#include "pages/SettingsPage.h"

UiApp::UiApp() {}
UiApp::~UiApp() {}

void UiApp::begin(UiRenderer* renderer, UiCallbacks cb) {
  _r = renderer;
  _cb = cb;

  if (_r) _r->begin();

  if (!_settings) _settings = new SettingsPage(_model, _mgr, this);
  if (!_main) _main = new IncubatorMainPage(_model, _mgr, _settings);

  _mgr.push(_main);
}

void UiApp::bindConfig(uint16_t* startYear,
                       uint8_t* startMonth,
                       uint8_t* startDay,
                       uint8_t* presetId,
                       int16_t* dayTempTable_x10,
                       int16_t* dayHumTable_x10,
                       int16_t* tempHyst_x10,
                       int16_t* humHyst_x10,
                       uint16_t* motorOnSec,
                       uint16_t* motorOffMin,
                       bool* heaterEnabled,
                       bool* motorEnabled,
                       bool* fanEnabled,
                       bool* humidifierEnabled) {
  _pStartYear = startYear;
  _pStartMonth = startMonth;
  _pStartDay = startDay;
  _pPresetId = presetId;
  _pTempHyst = tempHyst_x10;
  _pHumHyst = humHyst_x10;

  _pOnSec = motorOnSec;
  _pOffMin = motorOffMin;

  _pHeaterEn = heaterEnabled;
  _pMotorEn  = motorEnabled;
  _pFanEn    = fanEnabled;
  _pHumidEn  = humidifierEnabled;

  if (_settings) _settings->bindConfig(_pStartYear, _pStartMonth, _pStartDay, _pPresetId,
                                       dayTempTable_x10, dayHumTable_x10,
                                       _pTempHyst, _pHumHyst,
                                       _pOnSec, _pOffMin,
                                       _pHeaterEn, _pMotorEn, _pFanEn, _pHumidEn);

  syncModelFromConfig();
}

void UiApp::syncModelFromConfig() {
  if (!_pTempHyst) return;

  _model.startYear = *_pStartYear;
  _model.startMonth = *_pStartMonth;
  _model.startDay = *_pStartDay;

  _model.tempHyst_x10 = *_pTempHyst;
  _model.humHyst_x10 = *_pHumHyst;

  _model.motorOnSec = *_pOnSec;
  _model.motorOffMin = *_pOffMin;

  _model.heaterEnabled = *_pHeaterEn;
  _model.motorEnabled  = *_pMotorEn;
  _model.fanEnabled    = *_pFanEn;
  _model.humidifierEnabled = *_pHumidEn;
}

void UiApp::notifyConfigChanged() {
  syncModelFromConfig();
  if (_cb.onConfigChanged) _cb.onConfigChanged();
}

void UiApp::onEncoder(int delta) { _mgr.handleEncoder(delta); }
void UiApp::onClick() { _mgr.handleClick(); }
void UiApp::onLongPress() { _mgr.handleLongPress(); }
void UiApp::onVeryLongPress() { _mgr.handleVeryLongPress(); }

void UiApp::updateBlink(uint32_t nowMs) {
  if (nowMs - _lastBlinkMs >= 250) {
    _lastBlinkMs = nowMs;
    _model.blink = !_model.blink;
  }
}

void UiApp::tick(uint32_t nowMs) {
  updateBlink(nowMs);
  syncModelFromConfig();
}

void UiApp::render() {
  if (!_r) return;
  _mgr.render(*_r);
}

void UiApp::requestFactoryReset() { _factoryResetRequested = true; }
bool UiApp::takeFactoryResetRequest() {
  bool v = _factoryResetRequested;
  _factoryResetRequested = false;
  return v;
}

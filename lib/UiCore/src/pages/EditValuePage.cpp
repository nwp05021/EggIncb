#include <Arduino.h>
#include "EditValuePage.h"
#include "../PageManager.h"
#include "../UiApp.h"
#include "../UiRenderer.h"

static int32_t clamp32(int32_t v, int32_t lo, int32_t hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

EditValuePage::EditValuePage(PageManager& mgr, UiApp* app)
: _mgr(mgr), _app(app) {}

void EditValuePage::configure(const char* label,
                              const char* unit,
                              EditType type,
                              void* valuePtr,
                              int32_t min,
                              int32_t max,
                              int32_t step,
                              int32_t fastStep) {
  _label = label ? label : "";
  _unit  = unit ? unit : "";
  _type  = type;

  _min = min;
  _max = max;
  _step = (step == 0) ? 1 : step;
  _fastStep = (fastStep == 0) ? _step : fastStep;

  _pU8 = nullptr;
  _pU16 = nullptr;
  _pI16 = nullptr;
  _pB = nullptr;

  switch (_type) {
    case EditType::U8:      _pU8  = (uint8_t*)valuePtr; break;
    case EditType::U16:     _pU16 = (uint16_t*)valuePtr; break;
    case EditType::I16_X10: _pI16 = (int16_t*)valuePtr; break;
    case EditType::Bool:    _pB   = (bool*)valuePtr; break;
  }

  _value = readValue();
  _value = clamp32(_value, _min, _max);
}

int32_t EditValuePage::readValue() const {
  switch (_type) {
    case EditType::U8:      return _pU8 ? (int32_t)(*_pU8) : 0;
    case EditType::U16:     return _pU16 ? (int32_t)(*_pU16) : 0;
    case EditType::I16_X10: return _pI16 ? (int32_t)(*_pI16) : 0;
    case EditType::Bool:    return _pB ? (*_pB ? 1 : 0) : 0;
  }
  return 0;
}

void EditValuePage::writeValue(int32_t v) {
  v = clamp32(v, _min, _max);

  switch (_type) {
    case EditType::U8:
      if (_pU8) *_pU8 = (uint8_t)v;
      break;
    case EditType::U16:
      if (_pU16) *_pU16 = (uint16_t)v;
      break;
    case EditType::I16_X10:
      if (_pI16) *_pI16 = (int16_t)v;
      break;
    case EditType::Bool:
      if (_pB) *_pB = (v != 0);
      break;
  }

  _value = v;
  if (_app) _app->notifyConfigChanged();
}

void EditValuePage::onEnter() {
  _value = readValue();
  _value = clamp32(_value, _min, _max);
  _originalValue = _value;   // 원본 저장
}

void EditValuePage::onExit() {}

void EditValuePage::onEncoder(int delta) {
  if (delta == 0) return;
  int32_t step = _step;
  int32_t next = _value + (int32_t)delta * step;
  writeValue(next);
}

void EditValuePage::onClick() {
  // confirm
  _mgr.pop();
}

void EditValuePage::onLongPress() {
  // cancel
  writeValue(_originalValue);
  _mgr.pop();
}

void EditValuePage::onVeryLongPress() {
  // 동일 처리
  writeValue(_originalValue);
  _mgr.pop();
}

void EditValuePage::render(UiRenderer& r) {
  // Reuse renderer helper for edit page if present; otherwise render simple.
  // UiRenderer in this project already has drawEditValue(...) in older versions.
  bool blink = ((millis() / 300) % 2) == 0;
  bool isBool = (_type == EditType::Bool);
  bool isX10  = (_type == EditType::I16_X10);
  r.drawEditValue(_label, _unit, _value, blink, isBool, isX10);
}

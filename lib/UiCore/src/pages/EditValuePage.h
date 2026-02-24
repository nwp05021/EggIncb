#pragma once
#include "../Page.h"
#include <stdint.h>

class PageManager;
class UiApp;

// Type-safe edit target kinds
enum class EditType : uint8_t {
  U8,
  U16,
  I16_X10,   // fixed-point x10 (e.g. 375 => 37.5)
  Bool
};

class EditValuePage : public Page {
public:
  EditValuePage(PageManager& mgr, UiApp* app);

  // Note: valuePtr must match EditType:
  //  - U8      => uint8_t*
  //  - U16     => uint16_t*
  //  - I16_X10 => int16_t*
  //  - Bool    => bool*
  void configure(const char* label,
                 const char* unit,
                 EditType type,
                 void* valuePtr,
                 int32_t min,
                 int32_t max,
                 int32_t step,
                 int32_t fastStep);

  void onEnter() override;
  void onExit() override;
  void onEncoder(int delta) override;
  void onClick() override;
  void onLongPress() override;
  void onVeryLongPress() override;
  void render(class UiRenderer& r) override;

private:
  PageManager& _mgr;
  UiApp* _app;

  const char* _label = "";
  const char* _unit  = "";
  EditType _type = EditType::U16;

  // Target pointers (only one active at a time)
  uint8_t*  _pU8  = nullptr;
  uint16_t* _pU16 = nullptr;
  int16_t*  _pI16 = nullptr;
  bool*     _pB   = nullptr;

  int32_t _min = 0;
  int32_t _max = 0;
  int32_t _step = 1;
  int32_t _fastStep = 5;

  int32_t _value = 0;

  int32_t _originalValue = 0;

  int32_t readValue() const;
  void writeValue(int32_t v);
};

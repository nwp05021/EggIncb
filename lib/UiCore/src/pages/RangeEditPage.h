#pragma once

#include <stdint.h>

#include "../Page.h"

class UiApp;
class UiRenderer;
class PageManager;
struct UiModel;

// Range editor for schedule table (TEMP or HUM).
// - Select START/END
// - Edit VALUE
// - APPLY writes to the table and notifies config change

class RangeEditPage : public Page {
public:
  enum class Mode : uint8_t { Temp = 0, Hum = 1 };

  RangeEditPage(UiModel& m, PageManager& mgr, UiApp* app);

  void bindTable(int16_t* dayTempTable_x10, int16_t* dayHumTable_x10);
  void setMode(Mode mode);

  void onEnter() override;
  void onEncoder(int delta) override;
  void onClick() override;
  void onLongPress() override;
  void render(UiRenderer& r) override;

private:
  enum Item : uint8_t { Preset = 0, Start = 1, End = 2, Value = 3, Apply = 4, Back = 5, COUNT = 6 };

  enum class RangePreset : uint8_t {
    Custom = 0,
    D01_18,
    D19_21,
    D01_21,
    COUNT
  };

  void apply();
  void clampRange();
  void applyRangePreset(RangePreset p);
  const char* presetName(RangePreset p) const;
  int  stepForValue() const;
  void clampValue();
  const char* title() const;

  UiModel& _m;
  PageManager& _mgr;
  UiApp* _app;

  int16_t* _pTemp = nullptr;
  int16_t* _pHum  = nullptr;

  Mode _mode = Mode::Temp;

  uint8_t _startDay = 1;
  uint8_t _endDay   = 1;
  int16_t _value_x10 = 370; // default 37.0C or 37.0% depending mode

  RangePreset _preset = RangePreset::Custom;

  uint8_t _cursor = 0;
  uint8_t _page = 0;
  bool _editing = false;

  static constexpr int PAGE_SIZE = 4;
};

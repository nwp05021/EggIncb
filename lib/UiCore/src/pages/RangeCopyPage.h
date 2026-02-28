#pragma once

#include <stdint.h>

#include "../Page.h"

class UiApp;
class UiRenderer;
class PageManager;
struct UiModel;

// Copies a day-range from the schedule table into the shared clipboard.
class RangeCopyPage : public Page {
public:
  RangeCopyPage(UiModel& m, PageManager& mgr, UiApp* app);

  void bindTable(int16_t* dayTempTable_x10, int16_t* dayHumTable_x10);

  void onEnter() override;
  void onEncoder(int delta) override;
  void onClick() override;
  void onLongPress() override;
  void render(UiRenderer& r) override;

private:
  enum Item : uint8_t { Preset = 0, Start = 1, End = 2, Copy = 3, Back = 4, COUNT = 5 };

  enum class RangePreset : uint8_t {
    Custom = 0,
    D01_18,
    D19_21,
    D01_21,
    COUNT
  };

  void doCopy();
  void clampRange();
  void applyRangePreset(RangePreset p);
  const char* presetName(RangePreset p) const;

  UiModel& _m;
  PageManager& _mgr;
  UiApp* _app;

  int16_t* _pTemp = nullptr;
  int16_t* _pHum  = nullptr;

  uint8_t _startDay = 1;
  uint8_t _endDay   = 1;

  RangePreset _preset = RangePreset::Custom;

  uint8_t _cursor = 0;
  uint8_t _page = 0;
  bool _editing = false;
};

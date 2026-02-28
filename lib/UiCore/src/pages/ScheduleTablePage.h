#pragma once

#include "../Page.h"
#include <stdint.h>

class PageManager;
class UiApp;
class UiRenderer;
struct UiModel;

// Editable schedule table: D01-T, D01-H, ... D21-T, D21-H
class ScheduleTablePage : public Page {
public:
  ScheduleTablePage(UiModel& m, PageManager& mgr, UiApp* app);

  void bindTable(int16_t* dayTemp_x10, int16_t* dayHum_x10);

  void onEnter() override;
  void onEncoder(int delta) override;
  void onClick() override;
  void onLongPress() override;
  void render(UiRenderer& r) override;

private:
  UiModel& _m;
  PageManager& _mgr;
  UiApp* _app;

  int16_t* _pTemp = nullptr;
  int16_t* _pHum  = nullptr;

  int _cursor = 0;
  int _page = 0;

  static constexpr int PAGE_SIZE = 4;
  static constexpr int DAYS = 21;

  // Tools (top of list)
  static constexpr int TOOL_RANGE_TEMP = 0;
  static constexpr int TOOL_RANGE_HUM  = 1;
  static constexpr int TOOL_COPY       = 2;
  static constexpr int TOOL_PASTE      = 3;
  static constexpr int TOOL_PRESET_19_21_HUM_UP = 4;
  static constexpr int TOOL_COUNT      = 5;

  // items: tools + dayTemp/dayHum pairs + BACK
  static constexpr int ITEMS = TOOL_COUNT + (DAYS * 2) + 1; // + BACK

  class EditValuePage* _edit = nullptr;

  // tool pages
  class RangeEditPage* _rangeEdit = nullptr;
  class RangeCopyPage* _copy = nullptr;
  class RangePastePage* _paste = nullptr;
};

#pragma once

#include <stdint.h>

#include "../Page.h"

class UiApp;
class UiRenderer;
class PageManager;
struct UiModel;

// Pastes the shared clipboard into the schedule table starting at DEST day.
class RangePastePage : public Page {
public:
  RangePastePage(UiModel& m, PageManager& mgr, UiApp* app);

  void bindTable(int16_t* dayTempTable_x10, int16_t* dayHumTable_x10);

  void onEnter() override;
  void onEncoder(int delta) override;
  void onClick() override;
  void onLongPress() override;
  void render(UiRenderer& r) override;

private:
  enum Item : uint8_t { Dest = 0, Paste = 1, Back = 2, COUNT = 3 };

  void doPaste();

  UiModel& _m;
  PageManager& _mgr;
  UiApp* _app;

  int16_t* _pTemp = nullptr;
  int16_t* _pHum  = nullptr;

  uint8_t _destDay = 1;
  uint8_t _cursor = 0;
  bool _editing = false;
};

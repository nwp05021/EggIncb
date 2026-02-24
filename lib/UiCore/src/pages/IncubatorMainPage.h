#pragma once
#include "../Page.h"
#include "../UiModel.h"

class PageManager;

class IncubatorMainPage : public Page {
public:
  IncubatorMainPage(UiModel& m, PageManager& mgr, Page* settingsPage);

  void onEncoder(int delta) override;
  void onClick() override;
  void render(class UiRenderer& r) override;

private:
  UiModel& _m;
  PageManager& _mgr;
  Page* _settings;
};

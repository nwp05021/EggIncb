#pragma once
#include "../Page.h"
#include "../UiModel.h"

class PageManager;

class MainPage : public Page {
public:
  MainPage(UiModel& m, PageManager& mgr, class SettingsPage* settings);

  void onClick() override;
  void render(class UiRenderer& r) override;

private:
  UiModel& _m;
  PageManager& _mgr;
  SettingsPage* _settings;
};

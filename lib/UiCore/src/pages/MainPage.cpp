#include "MainPage.h"
#include "../UiRenderer.h"
#include "../PageManager.h"
#include <Arduino.h>
#include "SettingsPage.h"

MainPage::MainPage(UiModel& m, PageManager& mgr, SettingsPage* settings)
: _m(m), _mgr(mgr), _settings(settings) {}

void MainPage::onClick() {
  if (_settings) _mgr.push(_settings);
}

void MainPage::render(UiRenderer& r) {
  r.drawMain(_m, ::millis());
}

